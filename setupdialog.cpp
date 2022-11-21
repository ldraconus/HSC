#include "setupdialog.h"
#include "ui_setupdialog.h"

SetupDialog::SetupDialog(int size, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog)
{
    ui->setupUi(this);
    ui->spinBox->setMinimum(6);
    ui->spinBox->setMaximum(100);
    ui->spinBox->setValue(size);
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

int SetupDialog::size() {
    return ui->spinBox->value();
}
