#include "mainwindowqt.h"
#include "ui_mainwindowqt.h"

MainWindowQt::MainWindowQt(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowQt)
{
    ui->setupUi(this);
}

MainWindowQt::~MainWindowQt()
{
    delete ui;
}
