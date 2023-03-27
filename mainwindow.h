#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDomDocument>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QLabel>

class Char {
public:
    virtual QStringList disadvantages() = 0;
    virtual int         getPrimary(const QString&) = 0;
    virtual int         getPrimaryResistant(const QString s) = 0;
    virtual int         getSecondary(const QString&) = 0;
    virtual int         getSecondaryResistant(const QString s) = 0;
    virtual QByteArray  image() = 0;
    virtual void        load(QString) = 0;
    virtual QString     path() = 0;
    virtual QStringList martialArts() = 0;
    virtual QString     name() = 0;
    virtual QStringList perks() = 0;
    virtual QStringList powers() = 0;
    virtual QStringList skills() = 0;
    virtual QStringList talents() = 0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow* ref;

    QMap<QString, std::shared_ptr<Char>>& characters() { return _characters; }

    int     OK(const char*, const char*);
    int     OKCancel(const char*, const char*);
    int     Question(const char*, const char*, QFlags<QMessageBox::StandardButton>);
    int     Statement(const char*);
    int     YesNo(const char*, const char*);
    int     YesNoCancel(const char*, const char*);
    void    displayCharacter(QString);
    bool    getNextCharacter(int& current);
    void    removeChart(std::shared_ptr<Char>&);
    int     selectedRow();
    void    setCell(int row, int col, QString str);
    void    setChildrenFontSize(QObjectList children, int s);
    void    setCurrent(int);
    void    setFontSize(QWidget*, int size);
    void    setSegment(int, bool x = false);
    QString statToString(std::shared_ptr<Char>& chr, QString stat, bool m = false);
    void    updateChart();
    void    updateChart(std::shared_ptr<Char>&);
    void    updateComplications(std::shared_ptr<Char>&);
    void    updateMartialArts(std::shared_ptr<Char>&);
    void    updatePerks(std::shared_ptr<Char>&);
    void    updatePowers(std::shared_ptr<Char>&);
    void    updateSkills(std::shared_ptr<Char>&);
    void    updateStats(std::shared_ptr<Char>&);
    void    updateTalents(std::shared_ptr<Char>&);

    void keyReleaseEvent(QKeyEvent* e) override;
    void load(std::shared_ptr<Char>&, QString);

public slots:
    void delay();
    void doNew();
    void itemSelected();
    void load();
    void next();
    void remove();
#ifndef __wasm__
    void save();
#endif
    void setup();
    void start();

private:
    Ui::MainWindow *ui;
    QString _dir;

    QRect _ok          { -1, -1, 0, 0 };
    QRect _okcancel    { -1, -1, 0, 0 };
    QRect _question    { -1, -1, 0, 0 };
    QRect _statement   { -1, -1, 0, 0 };
    QRect _yesno       { -1, -1, 0, 0 };
    QRect _yesnocancel { -1, -1, 0, 0 };

    //   NAME
    QMap<QString, std::shared_ptr<Char>> _characters;
    //   DEX       SPD        NAME
    QMap<int, QMap<int, QList<QString>>> _chart;
    //   LABEL
    QMap<QString, QLabel*>               _displays;
    //   NAME
    QMap<QString, QList<bool>>           _delayed;

    int                                  _current = -1;
    int                                  _font = 9;
    int                                  _segment = -1;
    std::shared_ptr<class SimpleDialog>  _simpleDlg;
};

#endif // MAINWINDOW_H
