#ifndef SIMPLEDIALOG_H
#define SIMPLEDIALOG_H

#include <QDialog>

namespace Ui {
class SimpleDialog;
}

class SimpleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleDialog(QWidget *parent = nullptr);
    ~SimpleDialog();

    static std::shared_ptr<class Char> create();

private:
    Ui::SimpleDialog *ui;

public slots:
    void ok();
};

#endif // SIMPLEDIALOG_H
