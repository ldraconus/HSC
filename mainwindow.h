#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "character.h"

#include <QDomDocument>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int     OK(const char*, const char*);
    int     OKCancel(const char*, const char*);
    int     Question(const char*, const char*, QFlags<QMessageBox::StandardButton>);
    int     Statement(const char*);
    int     YesNo(const char*, const char*);
    int     YesNoCancel(const char*, const char*);
    QString calcDamageWithSTR(int, bool, bool);
    void    capture(int, QDomNode);
    void    captureAttr(int, QDomAttr);
    void    captureNode(int, QDomNode);
    void    captureData(int, QDomElement);
    void    displayCharacter(QString);
    bool    getNextCharacter(int& current);
    QString handleAdders(const QList<Character::Adder>&);
    QString handleList(Character::List&);
    QString handleModifiers(const QList<Character::Modifier>&);
    QString handleMultiPower(Character::MultiPower&);
    QString handlePower(Character::Power&);
    QString handleStat(Character::Stat&);
    QString handleSubPower(QList<Character::SubPower>&);
    QString handleVPP(Character::VariablePowerPool&);
    QString indent(int);
    void    removeChart(Character&);
    int     selectedRow();
    void    setCell(int row, int col, QString str);
    void    setChildrenFontSize(QObjectList children, int s);
    void    setCurrent(int);
    void    setFontSize(QWidget*, int size);
    void    setSegment(int, bool x = false);
    QString statToString(Character& chr, QString stat, bool m = false);
    void    updateChart();
    void    updateChart(Character&);
    void    updateComplications(Character&);
    void    updateMartialArts(Character&);
    void    updatePerks(Character&);
    void    updatePowers(Character&);
    void    updateSkills(Character&);
    void    updateStats(Character&);
    void    updateTalents(Character&);

    void keyReleaseEvent(QKeyEvent* e) override;

public slots:
    void delay();
    void itemSelected();
    void load();
    void next();
    void remove();
    void setup();
    void start();

private:
    Ui::MainWindow *ui;

    QRect _ok          { -1, -1, 0, 0 };
    QRect _okcancel    { -1, -1, 0, 0 };
    QRect _question    { -1, -1, 0, 0 };
    QRect _statement   { -1, -1, 0, 0 };
    QRect _yesno       { -1, -1, 0, 0 };
    QRect _yesnocancel { -1, -1, 0, 0 };

    //   NAME
    QMap<QString, Character>             _characters;
    //   DEX       SPD        NAME
    QMap<int, QMap<int, QList<QString>>> _chart;
    //   LABEL
    QMap<QString, QLabel*>               _displays;
    //   NAME
    QMap<QString, QList<bool>>           _delayed;

    int                                  _current = -1;
    int                                  _font = 9;
    int                                  _segment = -1;
};

#endif // MAINWINDOW_H
