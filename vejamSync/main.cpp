#include "main.h"

//http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
void vejamLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        localMsg.prepend("DEBUG: ");
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        localMsg.prepend("WARNING: ");
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        localMsg.prepend("CRITICAL: ");
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        localMsg.prepend("FATAL: ");
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }

    localMsg.append("\r\n");

    QFile f("vejam.debug.log");
    f.open(QIODevice::Append);
    f.write(localMsg);
    f.close();
}


static QtKApplicationParameters* g_appParameters = 0;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);  
    g_appParameters = new QtKApplicationParameters(0,QString("vejam"));

    if(g_appParameters->fileLoad(false))
    {
            setDefaultParameters();
            QString msg = "No se ha encontrado vejam.cfg!\r\nEstableciendo configuración por defecto.\r\n\r\n";
            msg += "vejam.cfg not found!\r\nSetting default configuration.";
#ifndef VEJAM_NO_GUI
            QMessageBox msgBox;
            msgBox.setText(msg);
            msgBox.exec();
#else
            qDebug() << msg;
#endif
			a.exit();
    }



    if(!g_appParameters->loadParam(QString("aplicacion"),QString("file-log"),0).compare("1"))
    {
        qInstallMessageHandler(vejamLogger);
    }

    authMachine gAuthMachine(g_appParameters);
    syncMachine gSyncMachine(g_appParameters);
    QObject::connect(&gAuthMachine,SIGNAL(authDone(QString)),&gSyncMachine, SLOT(OnAuthDone(QString)));
    gAuthMachine.authMachineSetState(authMachine::sstGoAuth);
    return a.exec();
}

void setDefaultParameters()
{
    g_appParameters->saveParam(QString("aplicacion"),QString("username"),QString("user@name.here"));
	g_appParameters->saveParam(QString("aplicacion"),QString("password"),QString("password"));
    g_appParameters->saveParam(QString("aplicacion"),QString("sync-interval"),QString("3600"));
    g_appParameters->saveParam(QString("aplicacion"),QString("cleanup-enable"),QString("1"));
    g_appParameters->saveParam(QString("aplicacion"),QString("file-log"),QString("0"));
    //g_appParameters->saveParam(QString("aplicacion"),QString("webkit-debug"),QString("0"));
    g_appParameters->saveParam(QString("aplicacion"),QString("streamming-mode"),QString("2")); //1: WebKit, 2: MJPEG
    g_appParameters->saveParam(QString("aplicacion"),QString("streamming-id"),QString("0")); //1...8
    g_appParameters->saveParam(QString("aplicacion"),QString("streamming-alias"),QString("RasPi Cam!")); //1...8
    g_appParameters->saveParam(QString("aplicacion"),QString("server-url"),QString("www.vejam.info/app-gui")); //http://www.vejam.info/app-gui/app-gui-welcome.html
    g_appParameters->saveParam(QString("conexion"),QString("webkit-port"),QString("40000"));
    g_appParameters->saveParam(QString("conexion"),QString("mjpeg-port"),QString("50000"));
	g_appParameters->saveParam(QString("conexion"),QString("mjpeg-uri"),QString("/?action=stream"));
    //g_appParameters->saveParam(QString("video"),QString("resolucion-x"),QString("320"));
    //g_appParameters->saveParam(QString("video"),QString("resolucion-y"),QString("240"));
    //g_appParameters->saveParam(QString("video"),QString("calidad"),QString("-1"));
    //g_appParameters->saveParam(QString("video"),QString("framerate-max"),QString("6"));
    //g_appParameters->saveParam(QString("device"),QString("selected"),QString("1"));	//Indica la camara per defecte.
    g_appParameters->fileSave();    
}
