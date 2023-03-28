#include "simpledialog.h"
#include "ui_simpledialog.h"

#include "mainwindow.h"

class simpleChar: public Char {
private:
    QString     charName;
    int         dcv;
    int         dex;
    int         dmcv;
    int         ed;
    int         ocv;
    int         omcv;
    int         pd;
    int         rEd;
    int         rPd;
    int         spd;
    QStringList empty;
    QByteArray  none;

public:
    virtual QStringList disadvantages()                      { return empty; }
    virtual int         getSecondaryResistant(const QString) { return 0; }
    virtual int         getSecondary(const QString&)         { return 0; }
    virtual QByteArray  image()                              { return none; }
    virtual QStringList martialArts()                        { return empty; }
    virtual QString     name()                               { return charName; }
    virtual QStringList perks()                              { return empty; }
    virtual QStringList powers()                             { return empty; }
    virtual QStringList skills()                             { return empty; }
    virtual QStringList talents()                            { return empty; }
    virtual QString     path()                               { return ""; }

    virtual int  getPrimary(const QString& s) {
        if (s == "DCV") return dcv;
        else if (s == "DEX") return dex;
        else if (s == "DMCV") return dmcv;
        else if (s == "ED") return ed;
        else if (s == "OCV") return ocv;
        else if (s == "OMCV") return omcv;
        else if (s == "PD") return pd;
        else if (s == "SPD") return spd;
        return 0;
    }
    virtual int  getPrimaryResistant(const QString s) {
        if (s == "PD") return rPd;
        else if (s == "ED") return rEd;
        return 0;
    };
    virtual void load(QString s) {
        QStringList data = s.split(',');
        charName = data[0];
        dcv  = data[1].toInt();
        dex  = data[2].toInt();
        dmcv = data[3].toInt();
        ed   = data[4].toInt();
        ocv  = data[5].toInt();
        omcv = data[6].toInt();
        pd   = data[7].toInt();
        rEd  = data[8].toInt();
        rPd  = data[9].toInt();
        spd  = data[10].toInt();
    }
};

SimpleDialog::SimpleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(ok()));
}

SimpleDialog::~SimpleDialog()
{
    delete ui;
}

void SimpleDialog::ok() {
    std::shared_ptr<Char> character;
    character = std::make_shared<simpleChar>();
    QString description = ui->name->text() + "," + ui->dcv->text() + "," + ui->dex->text() + "," +
                          ui->dmcv->text() + "," + ui->ed->text()  + "," + ui->ocv->text() + "," +
                          ui->omcv->text() + "," + ui->pd->text()  + "," + ui->rEd->text() + "," +
                          ui->rPd->text()  + "," + ui->spd->text();
    character->load(description);

    QString name = character->name();
    auto& chars = MainWindow::ref->characters();
    if (chars.find(name) != chars.end()) return;

    chars[name] = character;
    MainWindow::ref->updateChart(character);
    MainWindow::ref->displayCharacter(name);
}

std::shared_ptr<class Char> SimpleDialog::create() {
    std::shared_ptr<Char> character = std::make_shared<simpleChar>();
    return character;
}
