#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

namespace Ui {
class loadingDialog;
}

class loadingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit loadingDialog(QWidget *parent = 0);
    ~loadingDialog();
    void setProgress(quint16 progress);

private:
    Ui::loadingDialog *ui;
    QMovie *movie;
};

#endif // LOADINGDIALOG_H
