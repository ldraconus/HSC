#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setupdialog.h"

#include <iostream>

#include <QAction>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollArea>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction* loadAction = new QAction("Load");
    QAction* removeAction = new QAction("Remove");
    QAction* startAction = new QAction("Start");
    QAction* nextAction = new QAction("Next");
    QAction* delayAction = new QAction("Delay");
    QAction* setupAction = new QAction("Setup");

    ui->toolBar->addAction(loadAction);
    ui->toolBar->addAction(removeAction);
    ui->toolBar->addAction(startAction);
    ui->toolBar->addAction(nextAction);
    ui->toolBar->addAction(delayAction);
    ui->toolBar->addAction(setupAction);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===[DEBUG]-----------------------------------------------------------

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

static QString strDamage(int str) {
    QString dice;
    dice = QString("%1").arg(str / 5);
    if (str % 5 > 2) dice += "½";
    return dice + "d6";
}

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

QString MainWindow::calcDamageWithSTR(int dc, bool addStr, bool killing) {
    QString display;
    if (!addStr) {
        if (killing) display = calcKilling(dc, 0) + "K";
        else display = QString("%1d6").arg(dc);
    } else {
        QStringList fields = _displays["STR"]->text().split('\t');
        QString totals = fields[0];
        fields = totals.split('/');
        bool first = true;
        for (const QString& str: fields) {
            if (first) first = false;
            else display += "/";
            int newStr = str.toInt();
            if (killing) display += calcKilling(dc, newStr) + "K";
            else display += strDamage(dc * 5 + newStr);
        }
    }
    return display;
}

void MainWindow::capture(int ind, QDomNode n) {
    while (!n.isNull()) {
        captureNode(ind, n);
        n = n.nextSibling();
    }
}

void MainWindow::captureAttr(int ind, QDomAttr a) {
    std::cout << qPrintable(indent(ind)) << qPrintable(tr("|")) << qPrintable(a.name()) << qPrintable(tr(": ")) << qPrintable(a.value()) << std::endl;
}

void MainWindow::captureData(int ind, QDomElement elem) {
    capture(ind, elem.firstChild());
}

void MainWindow::captureNode(int ind, QDomNode n) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
        QString text = (e.tagName() == "CHARACTER_INFO") ? "" : e.text();
        if (text.isEmpty()) std::cout << qPrintable(indent(ind)) << qPrintable(e.tagName()) << std::endl;
        else std::cout << qPrintable(indent(ind)) << qPrintable(e.tagName()) << qPrintable(tr(": ")) << qPrintable(text) << std::endl;
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
    Character character = _characters[name];

    ui->name->setText(character.characterInfo.characterName);
    QPixmap pixmap;
    if (!character.image.image.isEmpty()) {
        pixmap.loadFromData(character.image.image);
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
        Character& character = _characters[name];
        QList<int> segments = phases(character.getPrimary("SPD") + character.getSecondary("SPD"));
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

QString MainWindow::handleList(Character::List& list) {
    QString display = list.attributes["ALIAS"];
    if (!list.attributes["INPUT"].isEmpty()) display += ": " + list.attributes["INPUT"];
    if (!list.attributes["NAME"].isEmpty()) display = list.attributes["NAME"] + ": " + display;
    display += handleAdders(list.specials.adders);
    display += handleModifiers(list.specials.modifiers);
    return display;
}

QString MainWindow::handleMultiPower(Character::MultiPower& mp) {
    QString display = mp.attributes["ALIAS"];
    if (!mp.attributes["INPUT"].isEmpty()) display += ": " + mp.attributes["INPUT"];
    if (!mp.attributes["NAME"].isEmpty()) display = mp.attributes["NAME"] + ": " + display;
    display += handleAdders(mp.specials.adders);
    display += handleModifiers(mp.specials.modifiers);
    return display;
}

QString MainWindow::handleAdders(const QList<Character::Adder>& adders) {
    QString display;
    for (const auto& adder: adders) {
        bool displayInString = true;
        if (adder.attributes.find("DISPLAYINSTRING") != adder.attributes.end()) displayInString = Character::Base::Bool(adder.attributes["DISPLAYINSTRING"]);
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

QString MainWindow::handleModifiers(const QList<Character::Modifier>& mods) {
    QString display;
    for (const auto& mod: mods) {
        bool displayInString = true;
        if (mod.attributes.find("DISPLAYINSTRING") != mod.attributes.end()) displayInString = Character::Base::Bool(mod.attributes["DISPLAYINSTRING"]);
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

QString MainWindow::handleSubPower(QList<Character::SubPower>& subPowers) {
    QString display;
    for (const auto& power: subPowers) {
        QString disp = power.attributes["ALIAS"];
        if (!power.attributes["INPUT"].isEmpty()) disp += ": " + power.attributes["INPUT"];
        if (!power.attributes["NAME"].isEmpty()) disp = power.attributes["NAME"] + ": " + disp;
        display += "; " + disp;
        int lvls = power.attributes["LEVELS"].toInt();
        bool dice = power.attributes.find("USESTANDARDEFFECT") != power.attributes.end();
        bool standard = false;
        if (dice) standard = Character::Base::Bool(power.attributes["USESTANDARDEFFECT"]);
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

QString MainWindow::handlePower(Character::Power& power) {
    QString display = power.attributes["ALIAS"];
    if (!power.attributes["INPUT"].isEmpty()) display += ": " + power.attributes["INPUT"];
    if (!power.attributes["NAME"].isEmpty()) display = power.attributes["NAME"] + ": " + display;
    int lvls = power.attributes["LEVELS"].toInt();
    bool dice = power.attributes.find("USESTANDARDEFFECT") != power.attributes.end();
    bool standard = false;
    if (dice) standard = Character::Base::Bool(power.attributes["USESTANDARDEFFECT"]);
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

QString MainWindow::handleStat(Character::Stat& stat) {
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
        if (inMultipower) display = QString("\t%1 ").arg(Character::Base::Bool(stat.attributes["ULTRA_SLOT"]) ? "(f)" : "(m)") + display;
        else display = "\t" + display;
    }
    return display;
}

QString MainWindow::handleVPP(Character::VariablePowerPool& vpp) {
    QString display = vpp.attributes["ALIAS"];
    if (!vpp.attributes["INPUT"].isEmpty()) display += ": " + vpp.attributes["INPUT"];
    if (!vpp.attributes["NAME"].isEmpty()) display = vpp.attributes["NAME"] + ": " + display;
    display += handleAdders(vpp.specials.adders);
    display += handleModifiers(vpp.specials.modifiers);
    return display;
}

QString MainWindow::indent(int ind) {
    QString str = "";
    for (int i = 0; i < ind; ++i) str += "  ";
    return str;
}

QString getRoll(const QString& stat, int levels) {
    QString display;
    QStringList fields = stat.split('\t');
    QString rolls = fields[1];
    fields = rolls.split('/');
    bool first = true;
    for (const QString& roll: fields) {
        if (first) first = false;
        else display += "/";
        int newRoll = roll.split('-')[0].toInt() + levels;
        display += QString("%1-").arg(newRoll);
    }
    return display;
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

QString MainWindow::statToString(Character& character, QString stat, bool m) {
    int primary = character.getPrimary(stat.toUpper());
    int secondary = primary + character.getSecondary(stat.toUpper());
    QString display;
    if (primary == secondary) display = QString("%1%2").arg(primary).arg(m ? "m" : "");
    else display = QString("%1%3/%2%3").arg(primary).arg(secondary).arg(m ? "m" : "");
    return display;
}

void MainWindow::removeChart(Character& character) {
    int DEX = character.getPrimary("DEX");
    DEX = DEX + character.getSecondary("DEX");
    int SPD = character.getPrimary("SPD");
    SPD = SPD + character.getSecondary("SPD");

    QMap<int, QMap<int, QList<QString>>>::const_iterator dex = _chart.constFind(DEX);
    QMap<int, QList<QString>>::const_iterator spd = dex->constFind(SPD);
    QList<QString>::const_iterator name;
    for (name = spd->begin(); *name != character.characterInfo.characterName; ++name);
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
                Character& chr = _characters[name];
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

void MainWindow::updateChart(Character& character) {
    int DEX = character.getPrimary("DEX");
    DEX = DEX + character.getSecondary("DEX");
    int SPD = character.getPrimary("SPD");
    SPD = SPD + character.getSecondary("SPD");

    _chart[DEX][SPD].append(character.characterInfo.characterName);
    updateChart();
}

void MainWindow::updateComplications(Character& character) {
    QGroupBox* complications = ui->complicationBox;
    QLayout* vbox = complications->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->complicationBox->setHidden(character.disadvantages.list.isEmpty());
    for (const auto& complication: character.disadvantages.list) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display = complication.attributes["ALIAS"];
        if (!complication.attributes["INPUT"].isEmpty()) display += ": " + complication.attributes["INPUT"];
        if (!complication.attributes["NAME"].isEmpty()) display = complication.attributes["NAME"] + ": " + display;
        display += handleAdders(complication.specials.adders);
        display += handleModifiers(complication.specials.modifiers);
        int parens = countOpens(display) - countCloses(display);
        for (int i = 0; i < parens; ++i) display += ")";
        label->setWordWrap(true);
        label->setText(display);
        vbox->addWidget(label);
    }
}

void MainWindow::updateMartialArts(Character& character) {
    QGroupBox* martialArts = ui->martialArtsBox;
    QLayout* vbox = martialArts->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->martialArtsBox->setHidden(character.martialArts.list.isEmpty());
    for (const auto& maneuver: character.martialArts.list) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display = maneuver.attributes["ALIAS"];
        if (!maneuver.attributes["INPUT"].isEmpty()) display += ": " + maneuver.attributes["INPUT"];
        if (!maneuver.attributes["NAME"].isEmpty()) display = maneuver.attributes["NAME"] + ": " + display;
        display += ": ";
        if (!maneuver.attributes["PHASE"].isEmpty()) display += half(maneuver.attributes["PHASE"]) + " Phase, ";
        if (!maneuver.attributes["OCV"].isEmpty()) display += maneuver.attributes["OCV"] + " OCV, ";
        if (!maneuver.attributes["DCV"].isEmpty()) display += maneuver.attributes["DCV"] + " DCV, ";
        int dc = maneuver.attributes["DC"].toInt();
        bool addStr = Character::Base::Bool(maneuver.attributes["ADDSTR"]);
        bool useWeapon = Character::Base::Bool(maneuver.attributes["USEWEAPON"]);
        if (useWeapon) display += replace(maneuver.attributes["WEAPONEFFECT"], "[WEAPONDC]", calcDamageWithSTR(dc, addStr, useWeapon));
        else display += replace(maneuver.attributes["EFFECT"], "[NORMALDC]", calcDamageWithSTR(dc, addStr, useWeapon));
        label->setWordWrap(true);
        label->setText(display);
        vbox->addWidget(label);
    }
}

void MainWindow::updatePerks(Character& character) {
    QGroupBox* perks = ui->perksBox;
    QLayout* vbox = perks->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->perksBox->setHidden(character.perks.list.isEmpty());
    for (const auto& perk: character.perks.list) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
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
        label->setWordWrap(true);
        label->setText(display);
        vbox->addWidget(label);
    }
}

void MainWindow::updatePowers(Character& character) {
    QGroupBox* powers = ui->powerBox;
    QLayout* vbox = powers->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->powerBox->setHidden(character.powers.items.isEmpty());
    for (auto& item: character.powers.items) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display;
        Character::Power* power = std::get_if<Character::Power>(&item);
        if (power != nullptr) display = handlePower(*power);
        else {
            Character::Stat* stat = std::get_if<Character::Stat>(&item);
            if (stat != nullptr) display = handleStat(*stat);
            else {
                Character::MultiPower* mp = std::get_if<Character::MultiPower>(&item);
                if (mp != nullptr) display = handleMultiPower(*mp);
                else {
                    Character::VariablePowerPool* vpp = std::get_if<Character::VariablePowerPool>(&item);
                    if (vpp != nullptr) display = handleVPP(*vpp);
                    else {
                        Character::List* list = std::get_if<Character::List>(&item);
                        if (list != nullptr) display = handleList(*list);
                    }
                }
            }
        }
        label->setWordWrap(true);
        label->setText(display);        
        vbox->addWidget(label);
    }
}

void MainWindow::updateSkills(Character& character) {
    QGroupBox* skills = ui->skillBox;
    QLayout* vbox = skills->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->skillBox->setHidden(character.skills.list.isEmpty());
    for (const auto& skill: character.skills.list) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display = skill.attributes["ALIAS"];
        if (!skill.attributes["INPUT"].isEmpty()) display += ": " + skill.attributes["INPUT"];
        if (!skill.attributes["NAME"].isEmpty()) display = skill.attributes["NAME"] + ": " + display;
        if (skill.attributes["LEVELSONLY"] == "Yes") display += " +" + skill.attributes["LEVELS"];
        else if (skill.attributes["FAMILIARITY"] == "Yes") display += " 8-";
        else if (skill.attributes["PROFICIENCY"] == "Yes") display += " 10-";
        else if (skill.attributes["CHARACTERISTIC"] == "GENERAL") display += QString(" %1-").arg(11 + skill.attributes["LEVELS"].toInt());
        else display += " " + getRoll(_displays[skill.attributes["CHARACTERISTIC"]]->text(), skill.attributes["LEVELS"].toInt());
        label->setWordWrap(true);
        label->setText(display);
        vbox->addWidget(label);
    }
}

void MainWindow::updateStats(Character& character) {
    QList<QString> stats {
        "STR", "DEX", "CON", "INT", "EGO", "PRE", "OCV", "DCV", "OMCV", "DMCV", "SPD",
        "PD", "ED", "REC", "END", "BODY", "STUN", "Running", "Swimming", "Leaping"
    };

    int count = 0;
    for (const auto& stat: stats) {
        QString display = statToString(character, stat, count > 16);
        int primary = character.getPrimary(stat.toUpper());
        int secondary = primary + character.getSecondary(stat.toUpper());
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
            if (primary == secondary) display += QString("\t%1 " + stat + " (%2 rPD)").arg(primary).arg(character.getPrimaryResistant(stat));
            else display += QString("\t%1/%2 " + stat + " (%3/%4 rPD)").arg(primary).arg(secondary).arg(character.getPrimaryResistant(stat)).arg(character.getSecondaryResistant(stat));
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

void MainWindow::updateTalents(Character& character) {
    QGroupBox* talents = ui->talentBox;
    QLayout* vbox = talents->layout();
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    ui->talentBox->setHidden(character.talents.list.isEmpty());
    for (const auto& talent: character.talents.list) {
        QLabel* label = new QLabel;
        setFontSize(label, _font);
        QString display = talent.attributes["ALIAS"];
        if (!talent.attributes["INPUT"].isEmpty()) display += ": " + talent.attributes["INPUT"];
        if (!talent.attributes["NAME"].isEmpty()) display = talent.attributes["NAME"] + ": " + display;
        label->setWordWrap(true);
        label->setText(display);
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
        Character character = _characters[name];
        int SPD = character.getPrimary("SPD");
        SPD = SPD + character.getSecondary("SPD");
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

void MainWindow::itemSelected() {
    int row = selectedRow();
    if (row == -1) return;
    QLabel* lbl = (QLabel*) ui->speedChart->cellWidget(row, 0);
    QString name = lbl->text();
    name = name.mid(1, name.length() - 2);
    displayCharacter(name);
}

void MainWindow::load() {
    QString filename = QFileDialog::getOpenFileName(nullptr, tr("Import HDC file"), QString(), tr("Hero System Characters (*.hdc)"));
    if (filename.isEmpty()) return;

    QDomDocument chr(filename);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    if (!chr.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    Character character;
    character.load(chr);

    QString name = character.characterInfo.characterName;
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
    Character character = _characters[name];
    QMap<QString, Character>::const_iterator chr = _characters.constFind(name);
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
    static QKeyCombination Remove(Qt::CTRL, Qt::Key_R);
    static QKeyCombination Start(Qt::CTRL,  Qt::Key_S);
    static QKeyCombination Delay(Qt::CTRL,  Qt::Key_D);
    static QKeyCombination Next(Qt::CTRL,   Qt::Key_N);
    static QKeyCombination Setup(Qt::ALT,   Qt::Key_S);

    if (e->keyCombination() == Load)        load();
    else if (e->keyCombination() == Remove) remove();
    else if (e->keyCombination() == Start)  start();
    else if (e->keyCombination() == Delay)  delay();
    else if (e->keyCombination() == Setup)  setup();
    else if (e->keyCombination() == Next)   next();
}
