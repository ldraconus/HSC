#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(int, QWidget *parent = nullptr);
    ~SetupDialog();

    int size();

private:
    Ui::SetupDialog *ui;
};

#endif // SETUPDIALOG_H
