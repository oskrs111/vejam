#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>

namespace Ui {
class mainDialog;
}

class mainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit mainDialog(QWidget *parent = 0);
    ~mainDialog();

private slots:    

    void on_okButton_clicked();

    void on_configButton_clicked();

private:
    Ui::mainDialog *ui;
};

#endif // MAINDIALOG_H
