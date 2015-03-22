#ifndef MAIN_H
#define MAIN_H
#include <QCoreApplication>
#include <QMessageLogContext>
#include "qtkapplicationparameters.h"
#include "authMachine.h"
#include "syncMachine.h"
#define VEJAM_APP_VERSION "BETA 1.0"

void vejamLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void setDefaultParameters();
#endif // MAIN_H
