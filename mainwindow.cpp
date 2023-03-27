#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setupdialog.h"

#include "simpledialog.h"

#include "hsccharacter.h"
#include "../../HSCCU/HSCCU/character.h"

#include <iostream>

#include <QAction>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollArea>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow* MainWindow::ref;

static QString strDamage(int str) {
    QString dice;
    dice = QString("%1").arg(str / 5);
    if (str % 5 > 2) dice += "½";
    return dice + "d6";
}

static QString calcKilling(int dc, int str) {
    int strDC = (str + 2) / 5;
    int num = (dc + strDC) / 3;
    int rem = dc % 3;
    switch (rem) {
    case 0: return QString("%1d6").arg(num);
    case 1: return QString("%1d6+1").arg(num);
    case 2: return QString("%1½d6").arg(num);
    }
    return "?";
}

static int count(QString str, QString what) {
    auto x = str.split(what);
    return x.count() - 1;
}

static int countOpens(QString str) {
    return count(str, "(");
}

static int countCloses(QString str) {
    return count(str, ")");
}

static QString half(QString x) {
    if (x == "1/2") return "½";
            return x;
}

static QString replace(QString str, QString var, QString val) {
    QStringList x = str.split(var);
    if (x.count() == 1) return str;
    return x[0] + val + x[1];
}

class hscChar: public Char {
private:
    hscCharacter character;

    void    capture(int, QDomNode);
    void    captureAttr(int, QDomAttr);
    void    captureNode(int, QDomNode);
    void    captureData(int, QDomElement);
    QString handleAdders(const QList<hscCharacter::Adder>&);
    QString handleList(hscCharacter::List&);
    QString handleModifiers(const QList<hscCharacter::Modifier>&);
    QString handleMultiPower(hscCharacter::MultiPower&);
    QString handlePower(hscCharacter::Power&);
    QString handleStat(hscCharacter::Stat&);
    QString handleSubPower(QList<hscCharacter::SubPower>&);
    QString handleVPP(hscCharacter::VariablePowerPool&);
    QString indent(int);

    QString calcDamageWithSTR(int dc, bool addStr, bool killing) {
        QString display;
        if (!addStr) {
            if (killing) display = calcKilling(dc, 0) + "K";
            else display = QString("%1d6").arg(dc);
        } else {
            int prim = getPrimary("STR");
            int sec  = getSecondary("STR") + prim;
            if (killing) display += calcKilling(dc, prim) + "K";
            else display += strDamage(dc * 5 + prim);
            if (sec != prim) {
                display += "/";
                if (killing) display += calcKilling(dc, sec) + "K";
                else display += strDamage(dc * 5 + sec);
            }
        }
        return display;
    }

    QString getRoll(const QString& stat, int levels) {
        QString display;
        int prim = getPrimary(stat);
        int sec  = getSecondary(stat) + prim;
        int primRoll = 9 + (prim + 2) / 5 + levels;
        int secRoll = 9 + (sec + 2) / 5 + levels;
        display = QString("%1-").arg(primRoll);
        if (prim != sec) display += "/" + QString("%1-").arg(secRoll);
        return display;
    }


public:
    int         getPrimary(const QString& s)           { return character.getPrimary(s); }
    int         getPrimaryResistant(const QString s)   { return character.getPrimaryResistant(s); }
    int         getSecondary(const QString& s)         { return character.getSecondary(s); }
    int         getSecondaryResistant(const QString s) { return character.getSecondaryResistant(s); }
    QByteArray  image()                                { return character.image.image; }
    QString     name()                                 { return character.characterInfo.characterName; }

    QStringList disadvantages() {
        QStringList disad;
        for (const auto& complication: character.disadvantages.list) {
            QString display = complication.attributes["ALIAS"];
            if (!complication.attributes["INPUT"].isEmpty()) display += ": " + complication.attributes["INPUT"];
            if (!complication.attributes["NAME"].isEmpty()) display = complication.attributes["NAME"] + ": " + display;
            display += handleAdders(complication.specials.adders);
            display += handleModifiers(complication.specials.modifiers);
            int parens = countOpens(display) - countCloses(display);
            for (int i = 0; i < parens; ++i) display += ")";
            disad.append(display);
        }
        return disad;
    }

    void load(QString f) {
        QDomDocument chr(f);
        QFile file(f);
        if (!file.open(QIODevice::ReadOnly)) return;
        if (!chr.setContent(&file)) {
            file.close();
            return;
        }
        file.close();
    }

    QStringList martialArts() {
        QStringList ma;
        for (const auto& maneuver: character.martialArts.list) {
            QString display = maneuver.attributes["ALIAS"];
            if (!maneuver.attributes["INPUT"].isEmpty()) display += ": " + maneuver.attributes["INPUT"];
            if (!maneuver.attributes["NAME"].isEmpty()) display = maneuver.attributes["NAME"] + ": " + display;
            display += ": ";
            if (!maneuver.attributes["PHASE"].isEmpty()) display += half(maneuver.attributes["PHASE"]) + " Phase, ";
            if (!maneuver.attributes["OCV"].isEmpty()) display += maneuver.attributes["OCV"] + " OCV, ";
            if (!maneuver.attributes["DCV"].isEmpty()) display += maneuver.attributes["DCV"] + " DCV, ";
            int dc = maneuver.attributes["DC"].toInt();
            bool addStr = hscCharacter::Base::Bool(maneuver.attributes["ADDSTR"]);
            bool useWeapon = hscCharacter::Base::Bool(maneuver.attributes["USEWEAPON"]);
            if (useWeapon) display += replace(maneuver.attributes["WEAPONEFFECT"], "[WEAPONDC]", calcDamageWithSTR(dc, addStr, useWeapon));
            else display += replace(maneuver.attributes["EFFECT"], "[NORMALDC]", calcDamageWithSTR(dc, addStr, useWeapon));
        }
        return ma;
    }

    QStringList perks() {
        QStringList perks;
        for (const auto& perk: character.perks.list) {
            QString display = perk.attributes["ALIAS"];
            if (!perk.attributes["INPUT"].isEmpty()) display += ": " + perk.attributes["INPUT"];
            if (!perk.attributes["NAME"].isEmpty()) display = perk.attributes["NAME"] + ": " + display;
            if (!perk.specials.adders.isEmpty()) {
                for (const auto& adder: perk.specials.adders) {
                    if (adder.attributes["OPTION"].toInt() != 0) display += " " + adder.attributes["OPTION_ALIAS"];
                    else display += " (" +  adder.attributes["OPTION_ALIAS"] + ")";
                }
                if (perk.attributes["LEVELS"].toInt() != 0) display += QString(", +%1/+%1d6").arg(perk.attributes["LEVELS"]);
            }
            perks.append(display);
        }
        return perks;
    }

    QStringList powers() {
        QStringList powers;
        for (auto& item: character.powers.items) {
            QString display;
            hscCharacter::Power* power = std::get_if<hscCharacter::Power>(&item);
            if (power != nullptr) display = handlePower(*power);
            else {
                hscCharacter::Stat* stat = std::get_if<hscCharacter::Stat>(&item);
                if (stat != nullptr) display = handleStat(*stat);
                else {
                    hscCharacter::MultiPower* mp = std::get_if<hscCharacter::MultiPower>(&item);
                    if (mp != nullptr) display = handleMultiPower(*mp);
                    else {
                        hscCharacter::VariablePowerPool* vpp = std::get_if<hscCharacter::VariablePowerPool>(&item);
                        if (vpp != nullptr) display = handleVPP(*vpp);
                        else {
                            hscCharacter::List* list = std::get_if<hscCharacter::List>(&item);
                            if (list != nullptr) display = handleList(*list);
                        }
                    }
                }
            }
            powers.append(display);
        }
        return powers;
    }

    QStringList skills() {
        QStringList skills;
        for (const auto& skill: character.skills.list) {
            QString display = skill.attributes["ALIAS"];
            if (!skill.attributes["INPUT"].isEmpty()) display += ": " + skill.attributes["INPUT"];
            if (!skill.attributes["NAME"].isEmpty()) display = skill.attributes["NAME"] + ": " + display;
            if (skill.attributes["LEVELSONLY"] == "Yes") display += " +" + skill.attributes["LEVELS"];
            else if (skill.attributes["FAMILIARITY"] == "Yes") display += " 8-";
            else if (skill.attributes["PROFICIENCY"] == "Yes") display += " 10-";
            else if (skill.attributes["CHARACTERISTIC"] == "GENERAL") display += QString(" %1-").arg(11 + skill.attributes["LEVELS"].toInt());
            else display += " " + getRoll(skill.attributes["CHARACTERISTIC"], skill.attributes["LEVELS"].toInt());
            skills.append(display);
        }
        return skills;
    }

    QStringList talents() {
        QStringList talents;
        for (const auto& talent: character.talents.list) {
            QString display = talent.attributes["ALIAS"];
            if (!talent.attributes["INPUT"].isEmpty()) display += ": " + talent.attributes["INPUT"];
            if (!talent.attributes["NAME"].isEmpty()) display = talent.attributes["NAME"] + ": " + display;
            talents.append(display);
        }
        return talents;
    }
};

class hsccuChar: public Char {
private:
    Character character;
    Option opt;

    int str2idx(QString s) {
        QMap<QString, int> data { { "STR", 0 }, { "DEX", 1 }, { "CON", 2 }, { "INT", 3 }, { "EGO", 4 }, { "PRE", 5 },
                                  { "OCV", 6 }, { "DCV", 7 }, { "OMCV", 8 }, { "DMCV", 9 }, { "SPD", 10 },
                                  { "PD", 11 }, { "ED", 12 }, { "REC", 13 }, { "END", 14 }, { "BODY", 15 }, { "STUN", 16 } };
        return data[s];
    }

public:
    QByteArray  image()                        { return character.imageData(); }
    QString     name()                         { return character.characterName(); }

    QStringList disadvantages() {
        QStringList disad;
        for (const auto& complication: character.complications()) disad.append(complication->description());
        return disad;
    }

    int getPrimaryResistant(const QString s) {
        if (s == "PD") return character.rPD();
        else if (s == "ED") return character.rED();
        return 0;
    }

    int getSecondaryResistant(const QString s) {
        if (s == "PD") return character.temprPD();
        else if (s == "ED") return character.temprED();
        return 0;
    }

    int getPrimary(const QString& s) {
        if (s == "RUNNING") return 12;
        else if (s == "LEAPING") return 4;
        else if (s == "SWIMMING") return 4;
        Characteristic c = character.characteristic(str2idx(s));
        return c.base() + c.primary();
    }
    int getSecondary(const QString& s) {
        if (s == "RUNNING") return character.running();
        else if (s == "LEAPING") return character.leaping();
        else if (s == "SWIMMING") return character.swimming();
        return character.characteristic(str2idx(s)).secondary();
    }

    void load(QString f) {
        character.load(opt, f);
        rebuildCharacter(character);
    }

    QStringList martialArts() {
        QStringList ma;
        for (const auto& skl: character.skillsTalentsOrPerks()) {
            if (skl->name() == "Martial Arts") {
                static QMap<QString, QString> table = {
                    { "Choke Hold", "½ Phase, -2 OCV, +0 DCV, Grab 1 limb, 2d6 NND" },
                    { "Defensive Strike", "½ Phase, +1 OCV, +3 DCV, STR strike" },
                    { "Killing Strike", "½ Phase, -2 OCV, +0 DCV, HKA ½d6" },
                    { "Legsweep", "½ Phase, +2 OCV, -1 DCV, STR+1d6, target falls" },
                    { "Martial Block", "½ Phase, +2 OCV, +2 DCV, Block, abort" },
                    { "Martial Disarm", "½ Phase, -1 OCV, -1 DCV, Disarm, +10 STR" },
                    { "Martial Dodge", "½ Phase, No OCV, +5 DCV, Dodge, abort" },
                    { "Martial Escape", "½ Phase, +0 OCV, +0 DCV, +15 STR vs. Grabs" },
                    { "Martial Grab", "½ Phase, -1 OCV, -1 DCV, Grab 2 Limbs, +10 STR" },
                    { "Martial Strike", "½ Phase, +0 OCV, +2 DCV, STR+2d6" },
                    { "Martial Throw", "½ Phase, +0 OCV, +1 DCV, STR+v/10, target falls" },
                    { "Nerve Strike", "½ Phase, -1 OCV, +1 DCV, 2d6 NND" },
                    { "Offensive Strike", "½ Phase, -2 OCV, +1 DCV, STR+4d6" },
                    { "Passing Strike", "½ Phase, +1 OCV, +0 DCV, STR+v/10, full move" },
                    { "Sacrifice Throw", "½ Phase, +2 OCV, +1 DCV, STR, both fall" }
                };
                QString d = skl->description(false).mid(skl->name().length() + 2);
                auto maneuvers = d.split(", ");
                for (const auto& m: maneuvers) {
                    int row = 0;
                    for (const auto& cell: ma) {
                        if (cell.startsWith(m)) break;
                        else row++;
                    }
                    if (row == ma.count()) ma.append(m + ", " + table[m]);
                }
            }
        }
        return ma;
    }

    QStringList perks() {
        QStringList perks;
        for (const auto& skl: character.skillsTalentsOrPerks()) {
            if (!skl->isPerk()) continue;
            perks.append(skl->description());
        }
        return perks;
    }

    QStringList powers() {
        QStringList powers;
        for (const auto& pwr: character.powersOrEquipment()) {
            powers.append(pwr->description(false));
            if (pwr->isFramework()) {
                QString descr;
                pwr->display(descr);
                QStringList x = descr.split('\n');
                for (const auto& d: x) if (!x.isEmpty()) powers.append(d);
            }
        }
        return powers;
    }

    void rebuildCharFromPowers(Character& character, QList<shared_ptr<Power>>& list) {
        for (const auto& power: list) {
            if (power == nullptr) continue;

            if (power->name() == "Skill" && power->skill()->name() == "Combat Luck") {
                if (power->skill()->place() == 1) {
                    character.rPD() = character.rPD() + power->skill()->rPD();
                    character.rED() = character.rED() + power->skill()->rED();
                } else if (power->skill()->place() == 2){
                    character.temprPD() = character.temprPD() + power->skill()->rPD();
                    character.temprED() = character.temprED() + power->skill()->rED();
                }
            } else if (power->name() == "Barrier") {
                if (power->place() == 1) {
                    character.rPD() = character.rPD() + power->rPD();
                    character.rED() = character.rED() + power->rED();
                } else if (power->place() == 2) {
                    character.temprPD() += power->rPD() + power->PD();
                    character.temprED() += power->rED() + power->ED();
                }
            } else if (power->name() == "Flash Defense") {
                character.FD() += power->FD();
            }
            else if (power->name() == "Mental Defense") {
                character.MD() += power->MD();
            }
            else if (power->name() == "Power Defense") {
                character.PowD() += power->PowD();
            } else if (power->name() == "Density Increase") {
                character.STR().secondary(character.STR().secondary() + power->str());
                character.rPD() += power->rPD();
                character.rED() += power->rED();
                if (power->hasModifier("Nonresistant Defense")) {
                    character.PD().secondary(character.PD().secondary() + power->PD());
                    character.ED().secondary(character.ED().secondary() + power->ED());
                }
            } else if (power->name() == "Resistant Defense") {
                if (power->place() == 1) {
                    character.rPD() = character.rPD() + power->rPD() + power->PD();
                    character.rED() = character.rED() + power->rED() + power->ED();
                } else if (power->place() == 2) {
                    character.temprPD() = character.temprPD() + power->rPD() + power->PD();
                    character.temprED() = character.temprED() + power->rED() + power->ED();
                }
            } else if (power->name() == "Growth") {
                auto& sm = power->growthStats();
                character.STR().secondary(character.STR().secondary() + sm._STR);
                character.CON().secondary(character.CON().secondary() + sm._CON);
                character.PRE().secondary(character.PRE().secondary() + sm._PRE);
                character.PD().secondary(character.PD().secondary() + sm._PD);
                character.ED().secondary(character.ED().secondary() + sm._ED);
                character.BODY().secondary(character.BODY().secondary() + sm._BODY);
                character.STUN().secondary(character.STUN().secondary() + sm._STUN);
                if (power->hasModifier("Resistant")) {
                    character.rPD() += sm._PD;
                    character.rED() += sm._ED;
                }
            } else if (power->name() == "Characteristics") {
                int put = power->characteristic(-1);
                if (put < 1) continue;
                for (int i = 0; i < 17; ++i) {
                    if (put == 1) character.characteristic(i).primary(character.characteristic(i).primary() + power->characteristic(i));
                    else          character.characteristic(i).secondary(character.characteristic(i).secondary() + power->characteristic(i));
                }
                if (power->hasModifier("Resistant")) {
                    if (put == 1) {
                        character.rPD() += power->characteristic(11);
                        character.rED() += power->characteristic(12);
                    } else {
                        character.temprPD() += power->characteristic(11);
                        character.temprED() += power->characteristic(12);
                    }
                }
            } else if (power->isFramework()) rebuildCharFromPowers(character, power->list());
        }
    }

    void rebuildMoveFromPowers(Character& character,
                               QList<shared_ptr<Power>>& list,
                               QMap<QString, int>& movements,
                               QMap<QString, QString>& units,
                               QMap<QString, int>& doubles) {
        for (const auto& power: list) {
            if (power == nullptr) continue;

            if (power->name() == "Growth") {
                auto& sm = power->growthStats();
                character.running() += sm._running;
            } else if (power->name() == "Running") {
                character.running() += power->move();
            } else if (power->name() == "Leaping") {
                character.leaping() += power->move();
            } else if (power->name() == "Swimming") {
                character.swimming() += power->move();
            } else if (power->name() == "FTL Travel" ||
                       power->name() == "Flight" ||
                       power->name() == "Swinging" ||
                       power->name() == "Teleportation" ||
                       power->name() == "Tunneling") {
                movements[power->name()] += power->move();
                units[power->name()] = power->units();
                doubles[power->name()] = power->doubling();
            } else if (power->isFramework()) {
                rebuildMoveFromPowers(character, power->list(), movements, units, doubles);
            }
        }
    }

    void rebuildCharacter(Character& character) {
        character.rPD() = 0;
        character.rED() = 0;
        character.temprPD() = 0;
        character.temprED() = 0;
        character.tempPD() = 0;
        character.tempED() = 0;
        character.FD() = 0;
        character.MD() = 0;

        for (const auto& skill: character.skillsTalentsOrPerks()) {
            if (skill == nullptr) continue;

            if (skill->name() == "Combat Luck") {
                if (skill->place() == 1) {
                    character.rPD() = character.rPD() + skill->rPD();
                    character.rED() = character.rED() + skill->rED();
                } else if (skill->place() == 2){
                    character.temprPD() = character.temprPD() + skill->rPD();
                    character.temprED() = character.temprED() + skill->rED();
                }
            }
        }

        for (int i = 0; i < 17; ++i) {
            character.characteristic(i).primary(0);
            character.characteristic(i).secondary(0);
        }

        character.running()  = 12;
        character.leaping()  = 4;
        character.swimming() = 4;

        QMap<QString, int> movements;
        QMap<QString, QString> units;
        QMap<QString, int> doubles;

        rebuildCharFromPowers(character, character.powersOrEquipment());
        rebuildMoveFromPowers(character, character.powersOrEquipment(), movements, units, doubles);
    }

    QStringList skills() {
        QStringList skills;
        for (const auto& skl: character.skillsTalentsOrPerks()) {
            if (!skl->isSkill()) continue;
            skills.append(skl->description(true));
        }
        return skills;
    }

    QStringList talents() {
        QStringList talents;
        for (const auto& skl: character.skillsTalentsOrPerks()) {
            if (!skl->isTalent()) continue;
            talents.append(skl->description(true));
        }
        return talents;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ref = this;

    QAction* newAction = new QAction("New");
    QAction* loadAction = new QAction("Open");
    QAction* removeAction = new QAction("Delete");
    QAction* startAction = new QAction("Start");
    QAction* nextAction = new QAction("Next");
    QAction* delayAction = new QAction("Delay");
    QAction* setupAction = new QAction("Setup");

    ui->toolBar->addAction(newAction);
    ui->toolBar->addAction(loadAction);
    ui->toolBar->addAction(removeAction);
    ui->toolBar->addAction(startAction);
    ui->toolBar->addAction(nextAction);
    ui->toolBar->addAction(delayAction);
    ui->toolBar->addAction(setupAction);

    connect(newAction,    SIGNAL(triggered()), this, SLOT(doNew()));
    connect(loadAction,   SIGNAL(triggered()), this, SLOT(load()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));
    connect(startAction,  SIGNAL(triggered()), this, SLOT(start()));
    connect(nextAction,   SIGNAL(triggered()), this, SLOT(next()));
    connect(delayAction,  SIGNAL(triggered()), this, SLOT(delay()));
    connect(setupAction,  SIGNAL(triggered()), this, SLOT(setup()));

    connect(ui->speedChart, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelected()));

    _displays["STR"] = ui->STRLabel;
    _displays["DEX"] = ui->DEXLabel;
    _displays["CON"] = ui->CONLabel;
    _displays["INT"] = ui->INTLabel;
    _displays["EGO"] = ui->EGOLabel;
    _displays["PRE"] = ui->PRELabel;
    _displays["OCV"] = ui->OCVLabel;
    _displays["DCV"] = ui->DCVLabel;
    _displays["OMCV"] = ui->OMCVLabel;
    _displays["DMCV"] = ui->DMCVLabel;
    _displays["SPD"] = ui->SPDLabel;
    _displays["PD"] = ui->PDLabel;
    _displays["ED"] = ui->EDLabel;
    _displays["REC"] = ui->RECLabel;
    _displays["END"] = ui->ENDLabel;
    _displays["BODY"] = ui->BODYLabel;
    _displays["STUN"] = ui->STUNLabel;
    _displays["Running"] = ui->RunningLabel;
    _displays["Swimming"] = ui->SwimmingLabel;
    _displays["Leaping"] = ui->LeapingLabel;

    _font = this->font().pointSize();
    setFontSize(this, _font);

    ui->speedChart->setHorizontalHeaderLabels({ "Name", "DEX", "SPD", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "OCV", "DCV", "OMCV", "DMCV", "PD", "ED" });
    setFontSize(ui->speedChart, _font);
    setFontSize(ui->name, (_font * 4 + 1) / 3);

    for (int i = 0; i < ui->speedChart->columnCount(); ++i) ui->speedChart->resizeColumnToContents(i);
    for (int i = 0; i < ui->speedChart->rowCount(); ++i) ui->speedChart->resizeRowToContents(i);

    Modifiers createModifiersTable;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===[DEBUG]-----------------------------------------------------------

//====[Message Box]================================================

static int showMsgBox(QMessageBox& msgBox, QRect& save) {
    QRect pos = msgBox.geometry();
    int width = pos.width();
    int height = pos.height();
    if (save.top() >= 0) {
        pos.setTop(save.top());
        if (save.left() >= 0) pos.setLeft(save.left());
        pos.setWidth(width);
        pos.setHeight(height);
        msgBox.setGeometry(pos);
    }
    int res = msgBox.exec();
    pos = msgBox.geometry();
    save.setLeft(pos.left());
    save.setTop(pos.top());
    return res;
}

int MainWindow::YesNo(const char* msg, const char* title) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(title ? title : "Are you sure?");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    return showMsgBox(msgBox, _yesno);
}

int MainWindow::YesNoCancel(const char* msg, const char* title) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(title ? title : "Are you really sure?");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    return showMsgBox(msgBox, _yesnocancel);
}

int MainWindow::OK(const char* msg, const char* title) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(title ? title : "Something has happened.");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    return showMsgBox(msgBox, _ok);
}

int MainWindow::OKCancel(const char* msg, const char* title) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(title ? title : "Something bad is about to happened.");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    return showMsgBox(msgBox, _okcancel);
}

int MainWindow::Question(const char* msg, const char* title, QFlags<QMessageBox::StandardButton> buttons) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(title ? title : "Are you sure?");
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(buttons);
    return showMsgBox(msgBox, _question);
}

int MainWindow::Statement(const char* msg) {
    QMessageBox msgBox;
    msgBox.setInformativeText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    return showMsgBox(msgBox, _statement);
}

//====[ Main Code ]=====================================================

void hscChar::capture(int ind, QDomNode n) {
    while (!n.isNull()) {
        captureNode(ind, n);
        n = n.nextSibling();
    }
}

void hscChar::captureAttr(int ind, QDomAttr a) {
    std::cout << qPrintable(indent(ind)) << qPrintable("|") << qPrintable(a.name()) << qPrintable(": ") << qPrintable(a.value()) << std::endl;
}

void hscChar::captureData(int ind, QDomElement elem) {
    capture(ind, elem.firstChild());
}

void hscChar::captureNode(int ind, QDomNode n) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
        QString text = (e.tagName() == "CHARACTER_INFO") ? "" : e.text();
        if (text.isEmpty()) std::cout << qPrintable(indent(ind)) << qPrintable(e.tagName()) << std::endl;
        else std::cout << qPrintable(indent(ind)) << qPrintable(e.tagName()) << qPrintable(": ") << qPrintable(text) << std::endl;
        if (e.hasAttributes()) {
            QDomNamedNodeMap named = e.attributes();
            int count = named.length();
            for (int i = 0; i < count; ++i) {
                QDomNode node = named.item(i);
                captureAttr(ind + 1, node.toAttr());
            }
        }
        captureData(ind + 1, e);
    }
}

void MainWindow::displayCharacter(QString name) {
    std::shared_ptr<Char> character = _characters[name];

    ui->name->setText(character->name());
    QPixmap pixmap;
    QByteArray img = character->image();
    if (!img.isEmpty()) {
        pixmap.loadFromData(img);
        QPixmap scaled = pixmap.scaledToWidth(ui->image->width());
        QRect rect = ui->image->geometry();
        rect.setHeight(scaled.height());
        ui->image->setGeometry(rect);
        ui->image->setPixmap(scaled);
    } else ui->image->setPixmap(pixmap);
    updateStats(character);
    updateSkills(character);
    updatePerks(character);
    updateTalents(character);
    updateMartialArts(character);
    updatePowers(character);
    updateComplications(character);
}

static QList<int> phases(int spd) {
    static QList<QList<int>> chart {
        { 0 }, { 7 }, { 6, 12 }, { 4, 8, 12 }, { 3, 6, 9, 12 }, { 3, 5, 8, 10, 12 }, { 2, 4, 6, 8, 10, 12 },
        { 2, 4, 6, 7, 9, 11, 12 }, { 2, 3, 5, 6, 8, 9, 11, 12 }, { 2, 3, 4, 6, 7, 8, 10, 11, 12 },
        { 2, 3, 4, 5, 6, 8, 9, 10, 11, 12 }, { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 },
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
    };
    return chart[spd];
}

static QList<bool> chart(int spd) {
    QList<bool> line;
    for (int i = 0; i < 12; i++) line.append(false);
    auto phase = phases(spd);
    for (const auto& x: phase) line[x - 1] = true;
    return line;
}


static QString strPhases(int spd) {
    static QList<QString> chart {
        "", "7", "6,12", "4,8,12", "3,6,9,12", "3,5,8,10,12", "2,4,6,8,10,12", "2,4,6,7,9,11,12", "2,3,5,6,8,9,11,12",
        "2,3,4,6,7,8,10,11,12", "2,3,4,5,6,8,9,10,11,12", "2,3,4,5,6,7,8,9,10,11,12", "1,2,3,4,5,6,7,8,9,10,11,12"
    };
    return chart[spd];
}

bool MainWindow::getNextCharacter(int& current) {
    for (int cur = current + 1; cur != ui->speedChart->rowCount(); ++cur) {
        QString name = ((QLabel*) ui->speedChart->cellWidget(cur, 0))->text();
        name = name.mid(1, name.length() - 2);
        std::shared_ptr<Char>& character = _characters[name];
        QList<int> segments = phases(character->getPrimary("SPD") + character->getSecondary("SPD"));
        for (const auto& x: segments) if (x == _segment) {
            current = cur;
            return true;
        }
        if (_delayed.find(name) != _delayed.end() && _delayed[name][_segment]) {
            current = cur;
            return true;
        }
    }
    return false;
}

QString hscChar::handleList(hscCharacter::List& list) {
    QString display = list.attributes["ALIAS"];
    if (!list.attributes["INPUT"].isEmpty()) display += ": " + list.attributes["INPUT"];
    if (!list.attributes["NAME"].isEmpty()) display = list.attributes["NAME"] + ": " + display;
    display += handleAdders(list.specials.adders);
    display += handleModifiers(list.specials.modifiers);
    return display;
}

QString hscChar::handleMultiPower(hscCharacter::MultiPower& mp) {
    QString display = mp.attributes["ALIAS"];
    if (!mp.attributes["INPUT"].isEmpty()) display += ": " + mp.attributes["INPUT"];
    if (!mp.attributes["NAME"].isEmpty()) display = mp.attributes["NAME"] + ": " + display;
    display += handleAdders(mp.specials.adders);
    display += handleModifiers(mp.specials.modifiers);
    return display;
}

QString hscChar::handleAdders(const QList<hscCharacter::Adder>& adders) {
    QString display;
    for (const auto& adder: adders) {
        bool displayInString = true;
        if (adder.attributes.find("DISPLAYINSTRING") != adder.attributes.end()) displayInString = hscCharacter::Base::Bool(adder.attributes["DISPLAYINSTRING"]);
        if (!displayInString) continue;
        if (!display.isEmpty()) display += ", ";
        display += adder.attributes["ALIAS"];
        if (!adder.attributes["INPUT"].isEmpty()) display += ": " + adder.attributes["INPUT"];
        if (!adder.attributes["NAME"].isEmpty()) display = adder.attributes["NAME"] + ": " + display;
        if (adder.attributes.find("OPTION_ALIAS") != adder.attributes.end()) display += "=" + adder.attributes["OPTION_ALIAS"];
    }
    if (display.isEmpty()) return "";
    return "; " + display;
}

QString hscChar::handleModifiers(const QList<hscCharacter::Modifier>& mods) {
    QString display;
    for (const auto& mod: mods) {
        bool displayInString = true;
        if (mod.attributes.find("DISPLAYINSTRING") != mod.attributes.end()) displayInString = hscCharacter::Base::Bool(mod.attributes["DISPLAYINSTRING"]);
        if (!displayInString) continue;
        if (!display.isEmpty()) display += ", ";
        display += mod.attributes["ALIAS"];
        if (!mod.attributes["INPUT"].isEmpty()) display += ": " + mod.attributes["INPUT"];
        if (!mod.attributes["NAME"].isEmpty()) display = mod.attributes["NAME"] + ": " + display;
        if (mod.attributes.find("OPTION_ALIAS") != mod.attributes.end()) display += "=" + mod.attributes["OPTION_ALIAS"];
        if (!mod.attributes["COMMENTS"].isEmpty()) display += " (" + mod.attributes["COMMENTS"] + ")";
    }
    if (display.isEmpty()) return "";
    return "; " + display;
}

QString hscChar::handleSubPower(QList<hscCharacter::SubPower>& subPowers) {
    QString display;
    for (const auto& power: subPowers) {
        QString disp = power.attributes["ALIAS"];
        if (!power.attributes["INPUT"].isEmpty()) disp += ": " + power.attributes["INPUT"];
        if (!power.attributes["NAME"].isEmpty()) disp = power.attributes["NAME"] + ": " + disp;
        display += "; " + disp;
        int lvls = power.attributes["LEVELS"].toInt();
        bool dice = power.attributes.find("USESTANDARDEFFECT") != power.attributes.end();
        bool standard = false;
        if (dice) standard = hscCharacter::Base::Bool(power.attributes["USESTANDARDEFFECT"]);
        QString XMLID = power.attributes["XMLID"];
        bool isMovement = (XMLID == "FLIGHT" || XMLID == "TELEPORT" || XMLID == "SWINGING" || XMLID == "STRETCHING");
        bool isSTR = XMLID == "TELEKINESIS";
        bool isREC = XMLID == "ENDURANCERESERVEREC";
        bool isDEF = (XMLID == "FORCEFIELD" || XMLID == "FORCEWALL");
        if (dice) {
            if (standard) display += QString("; (stndard, %1)").arg((lvls * 7) / 2);
            else display += QString("; (%1d6)").arg(lvls);
        } else if (isDEF) {
            int PD = power.attributes["PDLEVELS"].toInt();
            int ED = power.attributes["EDLEVELS"].toInt();
            if (PD != 0 && ED == 0) display += QString("; (%1 rPD)").arg(PD);
            else if (PD == 0 && ED != 0) display += QString("; (%1 rED)").arg(ED);
            else if (ED != 0 && PD != 0) display += QString("; (%1 rPD/%2 rED)").arg(PD).arg(ED);
        } else if (isMovement) display += QString("; (%1\")").arg(lvls);
        else if (isSTR) display += QString("; (%1 STR)").arg(lvls);
        else if (isREC) display += QString("; (%1 REC)").arg(lvls);
        display += handleAdders(power.specials.adders);
        display += handleModifiers(power.specials.modifiers);
    }
    return display;
}

QString hscChar::handlePower(hscCharacter::Power& power) {
    QString display = power.attributes["ALIAS"];
    if (!power.attributes["INPUT"].isEmpty()) display += ": " + power.attributes["INPUT"];
    if (!power.attributes["NAME"].isEmpty()) display = power.attributes["NAME"] + ": " + display;
    int lvls = power.attributes["LEVELS"].toInt();
    bool dice = power.attributes.find("USESTANDARDEFFECT") != power.attributes.end();
    bool standard = false;
    if (dice) standard = hscCharacter::Base::Bool(power.attributes["USESTANDARDEFFECT"]);
    QString XMLID = power.attributes["XMLID"];
    bool isMovement = (XMLID == "FLIGHT" || XMLID == "TELEPORT" || XMLID == "SWINGING" || XMLID == "STRETCHING");
    bool isSTR = XMLID == "TELEKINESIS";
    bool isEND = XMLID == "ENDURANCERESERVE";
    bool isDEF = (XMLID == "FORCEFIELD" || XMLID == "FORCEWALL");
    bool inSomething = power.attributes.find("PARENTID") != power.attributes.end();
    if (dice) {
        if (standard) display += QString("; (stndard, %1)").arg((lvls * 7) / 2);
        else display += QString("; (%1d6)").arg(lvls);
    } else if (isDEF) {
        int PD = power.attributes["PDLEVELS"].toInt();
        int ED = power.attributes["EDLEVELS"].toInt();
        if (PD != 0 && ED == 0) display += QString("; (%1 rPD)").arg(PD);
        else if (PD == 0 && ED != 0) display += QString("; (%1 rED)").arg(ED);
        else if (ED != 0 && PD != 0) display += QString("; (%1 rPD/%2 rED)").arg(PD).arg(ED);
    } else if (isMovement) display += QString("; (%1\")").arg(lvls);
    else if (isSTR) display += QString("; (%1 STR)").arg(lvls);
    else if (isEND) display += QString("; (%1 END)").arg(lvls);
    display += handleAdders(power.specials.adders);
    display += handleModifiers(power.specials.modifiers);
    display += handleSubPower(power.specials.subPowers);
    if (inSomething) {
        bool inMultipower = power.attributes.find("ULTRA_SLOT") != power.attributes.end();
        if (inMultipower) display = QString("\t%1 ").arg((power.attributes["ULTRA_SLOT"] == "Yes") ? "(f)" : "(m)") + display;
        else display = "\t" + display;
    }
    return display;
}

QString hscChar::handleStat(hscCharacter::Stat& stat) {
    QString display = stat.attributes["ALIAS"];
    if (!stat.attributes["INPUT"].isEmpty()) display += ": " + stat.attributes["INPUT"];
    if (!stat.attributes["NAME"].isEmpty()) display = stat.attributes["NAME"] + ": " + display;
    int lvls = stat.attributes["LEVELS"].toInt();
    bool inSomething = stat.attributes.find("PARENTID") != stat.attributes.end();
    display += QString("; (+%1)").arg(lvls);
    display += handleAdders(stat.specials.adders);
    display += handleModifiers(stat.specials.modifiers);
    if (inSomething) {
        bool inMultipower = stat.attributes.find("ULTRA_SLOT") != stat.attributes.end();
        if (inMultipower) display = QString("\t%1 ").arg(hscCharacter::Base::Bool(stat.attributes["ULTRA_SLOT"]) ? "(f)" : "(m)") + display;
        else display = "\t" + display;
    }
    return display;
}

QString hscChar::handleVPP(hscCharacter::VariablePowerPool& vpp) {
    QString display = vpp.attributes["ALIAS"];
    if (!vpp.attributes["INPUT"].isEmpty()) display += ": " + vpp.attributes["INPUT"];
    if (!vpp.attributes["NAME"].isEmpty()) display = vpp.attributes["NAME"] + ": " + display;
    display += handleAdders(vpp.specials.adders);
    display += handleModifiers(vpp.specials.modifiers);
    return display;
}

QString hscChar::indent(int ind) {
    QString str = "";
    for (int i = 0; i < ind; ++i) str += "  ";
    return str;
}

QString MainWindow::statToString(shared_ptr<Char>& character, QString stat, bool m) {
    int primary = character->getPrimary(stat.toUpper());
    int secondary = primary + character->getSecondary(stat.toUpper());
    QString display;
    if (primary == secondary) display = QString("%1%2").arg(primary).arg(m ? "m" : "");
    else display = QString("%1%3/%2%3").arg(primary).arg(secondary).arg(m ? "m" : "");
    return display;
}

void MainWindow::removeChart(shared_ptr<Char>& character) {
    int DEX = character->getPrimary("DEX");
    DEX = DEX + character->getSecondary("DEX");
    int SPD = character->getPrimary("SPD");
    SPD = SPD + character->getSecondary("SPD");

    QMap<int, QMap<int, QList<QString>>>::const_iterator dex = _chart.constFind(DEX);
    QMap<int, QList<QString>>::const_iterator spd = dex->constFind(SPD);
    QList<QString>::const_iterator name;
    for (name = spd->begin(); *name != character->name(); ++name);
    _chart[DEX][SPD].erase(name);
    if (_chart[DEX][SPD].isEmpty()) _chart[DEX].erase(spd);
    if (_chart[DEX].isEmpty()) _chart.erase(dex);
    updateChart();
}

int MainWindow::selectedRow() {
    auto range = ui->speedChart->selectedRanges();
    if (range.isEmpty()) return -1;
    return range[0].topRow();
}

void MainWindow::setCurrent(int current) {
    int cols = ui->speedChart->columnCount();
    if (_current != -1) {
        for (int i = 0; i < cols; ++i) {
            QLabel* itm = (QLabel*) (ui->speedChart->cellWidget(_current, i));
            itm->setStyleSheet("");
        }
    }
    if (current != -1) {
        for (int i = 0; i < cols; ++i) {
            QLabel* itm = (QLabel*) (ui->speedChart->cellWidget(current, i));
            itm->setStyleSheet("QLabel { background: green }");
        }
    }
    _current = current;
}

void MainWindow::setChildrenFontSize(QObjectList children, int s) {
    for (auto x: children) {
        QWidget* w = dynamic_cast<QWidget*>(x);
        if (w != nullptr) setFontSize(w, s);
    }
}

void MainWindow::setFontSize(QWidget* w, int s) {
    QFont font = w->font();

    QLabel* lbl = dynamic_cast<QLabel*>(w);
    if (lbl != nullptr) {
        if (lbl->objectName() == "name") {
            font.setPointSize((s * 4 + 1) / 3);
            w->setFont(font);
        } else {
            font.setPointSize(s);
            w->setFont(font);
        }
        return;
    }

    font.setPointSize(s);
    w->setFont(font);

    if (w->objectName() == "centralwidget") {
        setChildrenFontSize(w->children(), s);
        return;
    } else if (w->objectName() == "scrollAreaWidgetContents") {
        setChildrenFontSize(w->children(), s);
        return;
    }

    QScrollArea* scr = dynamic_cast<QScrollArea*>(w);
    if (scr != nullptr) {
        setChildrenFontSize(scr->children(), s);
        return;
    }


    QGroupBox* box = dynamic_cast<QGroupBox*>(w);
    if (box != nullptr) {
        setChildrenFontSize(box->children(), s);
        return;
    }

    MainWindow* mwn = dynamic_cast<MainWindow*>(w);
    if (mwn != nullptr) {
        setChildrenFontSize(mwn->children(), s);
        return;
    }

    QSplitter* spt = dynamic_cast<QSplitter*>(w);
    if (spt != nullptr) {
        QWidget* chd = w->findChild<QWidget*>("scrollAreaWidgetContents");
        setChildrenFontSize(chd->children(), s);
        chd = w->findChild<QWidget*>("speedBox");
        setFontSize(chd, s);
        return;
    }

    QTableWidget* tbl = dynamic_cast<QTableWidget*>(w);
    if (tbl != nullptr) {
        int rows = tbl->rowCount();
        int cols = tbl->columnCount();
        for (int c = 0; c < cols; ++c) {
            QTableWidgetItem* hdr = tbl->horizontalHeaderItem(c);
            if (hdr) hdr->setFont(font);
        }
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                QLabel* lbl = (QLabel*) ui->speedChart->cellWidget(r, c);
                if (lbl == nullptr) continue;
                lbl->setFont(font);
            }

        return;
    }

    QToolBar* tbr = dynamic_cast<QToolBar*>(w);
    if (tbr != nullptr) {
        auto actions = tbr->children();
        for (auto x: actions) {
            QAction* action = dynamic_cast<QAction*>(x);
            if (action == nullptr) continue;
            action->setFont(font);
        }
        return;
    }
}

void MainWindow::setSegment(int segment, bool force) {
    if (force || (_segment != -1 && _segment != segment))
        for (int i = 0; i < ui->speedChart->rowCount(); ++i) {
            QLabel* itm = (QLabel*) (ui->speedChart->cellWidget(i, _segment + 2));
            itm->setStyleSheet("");
        }
    if (!force && segment != -1) {
        for (int i = 0; i < ui->speedChart->rowCount(); ++i) {
            QLabel* itm = (QLabel*) (ui->speedChart->cellWidget(i, segment + 2));
            itm->setStyleSheet("QLabel { background: green }");
        }
    }
    _segment = segment;
}

void MainWindow::setCell(int row, int col, QString str) {
    QLabel* lbl = new QLabel(" " + str + " ");
    setFontSize(lbl, _font);
    if (col >= 1) lbl->setAlignment(Qt::AlignCenter);
    else lbl->setAlignment(Qt::AlignVCenter);
    ui->speedChart->setCellWidget(row, col, lbl);
}

void MainWindow::updateChart() {
    int count = _characters.count();

    auto range = ui->speedChart->selectedRanges();
    if (!range.isEmpty()) ui->speedChart->setRangeSelected(range[0], false);
    ui->speedChart->setRowCount(0);
    ui->speedChart->update();
    ui->speedChart->setRowCount(count);
    ui->speedChart->update();

    int row = 0;
    QList<int> dexCount = _chart.keys();
    for (int i = dexCount.count(); i != 0 ; --i) {
        QMap<int, QList<QString>>& speeds = _chart[dexCount[i - 1]];
        QList<int> spdCount = speeds.keys();
        for (int j = spdCount.count(); j != 0 ; --j) {
            QList<QString>& chars = speeds[spdCount[j - 1]];
            for (int k = 0; k < chars.count(); ++k) {
                QString name = chars[k];
                std::shared_ptr<Char>& chr = _characters[name];
                setCell(row, 0, name);
                setCell(row, 1, statToString(chr, "DEX"));
                setCell(row, 2, statToString(chr, "SPD"));
                QList<bool> phase = chart(spdCount[j - 1]);
                for (int m = 0; m < phase.count(); ++m) {
                    QString X = "X";
                    if (!phase[m]) X = "";
                    if (X == "" && _delayed.find(name) != _delayed.end() && _delayed[name][m]) X = "D";
                    setCell(row, m + 3, X);
                }
                int col = 15;
                for (const auto& x: QStringList { "OCV", "DCV", "OMCV", "DMCV", "PD", "ED" }) setCell(row, col++, statToString(chr, x));
                row++;
            }
        }
    }

    int cols = ui->speedChart->columnCount();
    for (int i = 0; i < cols; ++i) ui->speedChart->resizeColumnToContents(i);
    for (int i = 0; i < count; ++i) ui->speedChart->resizeRowToContents(i);
}

void MainWindow::updateChart(std::shared_ptr<Char>& character) {
    int DEX = character->getPrimary("DEX");
    DEX = DEX + character->getSecondary("DEX");
    int SPD = character->getPrimary("SPD");
    SPD = SPD + character->getSecondary("SPD");

    _chart[DEX][SPD].append(character->name());
    updateChart();
}

void MainWindow::updateComplications(std::shared_ptr<Char>& character) {
    QGroupBox* complications = ui->complicationBox;
    QLayout* vbox = complications->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->complicationBox->setHidden(character->disadvantages().isEmpty());
    for (const auto& complication: character->disadvantages()) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        label->setWordWrap(true);
        label->setText(complication);
        vbox->addWidget(label);
    }
}

void MainWindow::updateMartialArts(std::shared_ptr<Char>& character) {
    QGroupBox* martialArts = ui->martialArtsBox;
    QLayout* vbox = martialArts->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const auto& ma = character->martialArts();
    ui->martialArtsBox->setHidden(ma.isEmpty());
    for (const auto& maneuver: ma) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display = maneuver;
        label->setWordWrap(true);
        label->setText(display);
        vbox->addWidget(label);
    }
}

void MainWindow::updatePerks(std::shared_ptr<Char>& character) {
    QGroupBox* perks = ui->perksBox;
    QLayout* vbox = perks->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const auto& prks = character->perks();
    ui->perksBox->setHidden(prks.isEmpty());
    for (const auto& perk: prks) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        label->setText(perk);
        vbox->addWidget(label);
    }
}

void MainWindow::updatePowers(std::shared_ptr<Char>& character) {
    QGroupBox* powers = ui->powerBox;
    QLayout* vbox = powers->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const auto& pwrs = character->powers();
    ui->powerBox->setHidden(pwrs.isEmpty());
    for (auto& item: pwrs) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        label->setWordWrap(true);
        label->setText(item);
        vbox->addWidget(label);
    }
}

void MainWindow::updateSkills(std::shared_ptr<Char>& character) {
    QGroupBox* skills = ui->skillBox;
    QLayout* vbox = skills->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const auto& skls = character->skills();
    ui->skillBox->setHidden(skls.isEmpty());
    for (const auto& skill: skls) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        label->setWordWrap(true);
        label->setText(skill);
        vbox->addWidget(label);
    }
}

void MainWindow::updateStats(std::shared_ptr<Char>& character) {
    QList<QString> stats {
        "STR", "DEX", "CON", "INT", "EGO", "PRE", "OCV", "DCV", "OMCV", "DMCV", "SPD",
        "PD", "ED", "REC", "END", "BODY", "STUN", "Running", "Swimming", "Leaping"
    };

    int count = 0;
    for (const auto& stat: stats) {
        QString display = statToString(character, stat, count > 16);
        int primary = character->getPrimary(stat.toUpper());
        int secondary = primary + character->getSecondary(stat.toUpper());
        if (count < 6) {
            if (primary == secondary) display += QString("\t%1-").arg(9 + (primary + 2) / 5);
            else display += QString("\t%1-/%2-").arg(9 + (primary + 2) / 5).arg(9 + (secondary + 2) / 5);
        } else display += "\t";
        if (stat == "STR") {
            if (primary == secondary) display += QString("\tHTH Damage %1 END[%2]").arg(strDamage(primary)).arg((primary + 5) / 10);
            else display += QString("\tHTH Damage %1/%2 END [%3/%4]").arg(strDamage(primary), strDamage(secondary)).arg((primary + 5) / 10).arg((secondary + 5) / 10);
        } else if (stat == "INT") {
            if (primary == secondary) display += QString("\tPER Roll %1-").arg(9 + (primary + 2) / 5);
            else display += QString("\tPER Roll %1-/%2-").arg(9 + (primary + 2) / 5).arg(9 + (secondary + 2) / 5);
        } else if (stat == "PRE") {
            if (primary == secondary) display += QString("\tPRE Attack: %1").arg(strDamage(primary));
            else display += QString("\tPRE Attack %1/%2").arg(strDamage(primary), strDamage(secondary));
        } else if (stat == "SPD") {
            if (primary == secondary) display += QString("\tPhases: %1").arg(strPhases(primary));
            else display += QString("\tPhases: %1/%2").arg(strPhases(primary), strPhases(secondary));
        } else if (stat == "PD" || stat == "ED") {
            if (primary == secondary) display += QString("\t%1 " + stat + " (%2 rPD)").arg(primary).arg(character->getPrimaryResistant(stat));
            else display += QString("\t%1/%2 " + stat + " (%3/%4 rPD)").arg(primary).arg(secondary).arg(character->getPrimaryResistant(stat)).arg(character->getSecondaryResistant(stat));
        } else if (stat == "Running") {
            if (primary == secondary) display += QString("\tEND [%1]").arg((primary + 4) / 10);
            else display += QString("\tEND[%1/%2]").arg((primary + 4) / 10).arg((secondary + 4) / 10);
        } else if (stat == "Swimming") {
            if (primary == secondary) display += QString("\tEND [%1]").arg(((primary - 3) / 2 + 5) / 10);
            else display += QString("\tEND[%1/%2]").arg(((primary - 3) / 2 + 5) / 10).arg(((secondary - 3) / 2 + 5) / 10);
        } else if (stat == "Leaping") {
            if (primary == secondary) display += QString("\t%1m forward, %2m upward").arg(primary).arg(primary / 2);
            else display += QString("\t%1m/%2m forward, %3m/%4m upward").arg(primary).arg(secondary).arg(primary / 2).arg(secondary / 2);
        }

        _displays[stat]->setText(display);
        count++;
    }
}

void MainWindow::updateTalents(std::shared_ptr<Char>& character) {
    QGroupBox* talents = ui->talentBox;
    QLayout* vbox = talents->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const auto& tlnts = character->talents();
    ui->talentBox->setHidden(tlnts.isEmpty());
    for (const auto& talent: tlnts) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        label->setWordWrap(true);
        label->setText(talent);
        vbox->addWidget(label);
    }
}

// ===[SLOTS]-----------------------------------------------------------

void MainWindow::delay() {
    if (_characters.isEmpty()) {
        OK("You need to load some characters\nbefore you can delay one", "Whoops!");
        return;
    }
    if (_segment == -1) {
        OK("You need to be in combat before\nyou can delay a character", "Whoops!");
        return;
    }

    QString name = ((QLabel*) ui->speedChart->cellWidget(_current, 0))->text();
    name = name.mid(1, name.length() - 2);
    if (_delayed.find(name) != _delayed.end()) {
        auto x = _delayed.constFind(name);
        _delayed.erase(x);
    } else {
        const auto& character = _characters[name];
        int SPD = character->getPrimary("SPD");
        SPD = SPD + character->getSecondary("SPD");
        QList<bool> spd = chart(SPD);
        int seg = _segment;
        if (seg == 12) seg = 0;
        QList<bool> delay;
        for (int i = 0; i < 12; ++i) delay.append(false);
        int count = 0;
        while (!spd[seg]) {
            delay[seg] = true;
            seg++;
            count++;
            if (seg == 12) seg = 0;
        }
        if (count == 0) {
            OK("You can't really delay if you move\nin the next segment", "Whoops!");
            return;
        }
        _delayed[name] = delay;
    }

    updateChart();
    setCurrent(_current);
    setSegment(_segment);
}

void MainWindow::doNew() {
    _simpleDlg = std::make_shared<SimpleDialog>();
    _simpleDlg->open();
}

void MainWindow::itemSelected() {
    int row = selectedRow();
    if (row == -1) return;
    QLabel* lbl = (QLabel*) ui->speedChart->cellWidget(row, 0);
    QString name = lbl->text();
    name = name.mid(1, name.length() - 2);
    displayCharacter(name);
}

void MainWindow::load() {
    QString filename = QFileDialog::getOpenFileName(nullptr, tr("Import Character file"), QString(), tr("Hero System Characters (*.hdc *.hsccu)"));
    if (filename.isEmpty()) return;

    std::shared_ptr<Char> character;
    if (filename.endsWith(".hsc")) character = make_shared<hscChar>();
    else character = make_shared<hsccuChar>();
    character->load(filename);

    QString name = character->name();
    if (_characters.find(name) != _characters.end()) return;

    _characters[name] = character;
    updateChart(character);
    displayCharacter(name);
}

void MainWindow::next() {
    if (_characters.isEmpty()) {
        OK("You need to load some characters\nbefore combat can continue!", "Whoops!");
        return;
    } else if (_segment == -1) {
        OK("You need to start combat before\nYou can move to the character!", "Whoops!");
        return;
    }
    int current = _current;
    if (!getNextCharacter(current)) {
        setSegment(_segment, true);
        if (_segment == 12) {
            OK("Post-Twelve Recoveries!", "Notice!");
            _segment = 1;
        } else ++_segment;
        current = -1;
        while (!getNextCharacter(current)) {
            ++_segment;
        }
    }
    setCurrent(current);
    setSegment(_segment);
}

void MainWindow::remove() {
    if (_characters.isEmpty()) {
        OK("You need to load some characters\nbefore you can delete one", "Whoops!");
        return;
    }
    int row = selectedRow();
    if (row == -1) {
        OK("You need to select a character\nbefore you can delete one", "Whoops!");
        return;
    }

    QString name = ((QLabel*) ui->speedChart->cellWidget(row, 0))->text();
    name = name.mid(1, name.length() - 2);
    auto& character = _characters[name];
    QMap<QString, std::shared_ptr<Char>>::const_iterator chr = _characters.constFind(name);
    _characters.erase(chr);
    auto range = ui->speedChart->selectedRanges();
    if (!range.isEmpty()) ui->speedChart->setRangeSelected(range[0], false);
    removeChart(character);
}

void MainWindow::setup() {
    _font = ui->BODYLabel->font().pointSize();
    SetupDialog dlg(_font);
    if (dlg.exec() == QDialog::Rejected) return;
    _font = dlg.size();

    setFontSize(this, _font);
    setFontSize(ui->name, (_font * 4 + 1) / 3);

    for (int i = 0; i < ui->speedChart->columnCount(); ++i) ui->speedChart->resizeColumnToContents(i);
    for (int i = 0; i < ui->speedChart->rowCount(); ++i) ui->speedChart->resizeRowToContents(i);
}

void MainWindow::start() {
    if (_characters.isEmpty()) {
        OK("You need to load some characters\nbefore combat can start!", "Whoops!");
        return;
    }
    setSegment(12);
    int current = -1;
    if (!getNextCharacter(current)) {
        setSegment(7);
        getNextCharacter(current);
    }
    setCurrent(current);
    setSegment(_segment);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
    static QKeyCombination Load(Qt::CTRL,   Qt::Key_L);
    static QKeyCombination New(Qt::CTRL,    Qt::Key_N);
    static QKeyCombination Remove(Qt::CTRL, Qt::Key_R);
    static QKeyCombination Start(Qt::CTRL,  Qt::Key_S);
    static QKeyCombination Delay(Qt::CTRL,  Qt::Key_D);
    static QKeyCombination Next(Qt::CTRL,   Qt::Key_N);
    static QKeyCombination Setup(Qt::ALT,   Qt::Key_S);

    if (e->keyCombination() == Load)        load();
    else if (e->keyCombination() == New)    doNew();
    else if (e->keyCombination() == Remove) remove();
    else if (e->keyCombination() == Start)  start();
    else if (e->keyCombination() == Delay)  delay();
    else if (e->keyCombination() == Setup)  setup();
    else if (e->keyCombination() == Next)   next();
}
