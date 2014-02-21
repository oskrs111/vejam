#include "loadingdialog.h"
#include "ui_loadingdialog.h"
#include <QMovie>

loadingDialog::loadingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadingDialog)
{
    ui->setupUi(this);
    this->movie = new QMovie(":/gif/img/ajax-loader.gif");
    ui->label->setMovie(this->movie);
    this->movie->start();
    this->setWindowFlags(Qt::SplashScreen);
    this->setProgress(0);
}

loadingDialog::~loadingDialog()
{
    delete ui;
    delete this->movie;
}

void loadingDialog::setProgress(quint16 progress)
{
    ui->progresslabel->setText(QString("%1").arg(progress));
}
