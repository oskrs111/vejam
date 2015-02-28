#include "mainwindow.h"
#ifdef VEJAM_GUI_QT_TYPE
#include "ui_mainwindowqt.h"
#else
#include "ui_mainwindow.h"
#include <QtWebKitWidgets>
#endif
#include <QCryptographicHash>
#include <QJsonObject>
#include <QMessageBox>
#include <QProcess>
#include <QBuffer>
#include <QEvent>
#include <QUrl>

#define VEJAM_APP_VERSION "BETA 1.0"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    #ifdef VEJAM_GUI_QT_TYPE
    ui(new Ui::MainWindowQt)
    #else
    ui(new Ui::MainWindow)
#endif
{
    ui->setupUi(this);   
    qDebug() << "MainWindow::MainWindow()";

    this->m_state = 0;
//    this->m_camera = 0;
//    this->m_viewfinder = 0;
//    this->m_imageCapture = 0;
    this->m_imageReady = false;
    this->m_frameInterval = 0;
    this->m_trayIcon = 0;
    this->m_websockServer = 0;
    this->p_ld = 0;
	this->p_manager = 0;
	this->m_webInterfaceLoadTryouts = 0;

    /*
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowSystemMenuHint;
    flags |= Qt::Widget;
    this->setWindowFlags(flags);
    this->setParent(0); // Create TopLevel-Widget
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WA_PaintOnScreen); // as pointed by Caveman (thanks!)
    */

    this->m_appParameters = new QtKApplicationParameters(this,QString("vejam"));
    if(this->m_appParameters->fileLoad(false))
    {      
		this->setDefaultParameters();
		 QMessageBox msgBox;  
		 QString msg = "No se ha encontrado vejam.cfg!\r\nEstableciendo configuración por defecto.\r\n\r\n";
		 msg += "vejam.cfg not found!\r\nSetting default configuration.";
         msgBox.setText(msg);
		 msgBox.setWindowIcon(QIcon(QPixmap(":/png/img/vejam_toolbar_h48.png")));
         msgBox.exec();
    }

    this->m_videoServer = new QtkVideoServer(this->m_appParameters, this);
    this->m_httpServer = new QtkHttpServer(this->loadParam(QString("conexion"),QString("mjpeg-port")).toInt(0,10), this);
    this->m_httpServer->setVideoServer(this->m_videoServer);

    if(this->m_videoServer->loadAvaliableCameras())
	{
		QMessageBox msgBox;
		QString msg = "No hay camaras disponibles?\r\n\r\n";
		msg = "No cameras avaliable?";
		msgBox.setText(msg);
		msgBox.setWindowIcon(QIcon(QPixmap(":/png/img/vejam_toolbar_h48.png")));
		msgBox.exec();
	}
	else
	{
		this->m_videoServer->startServer();
		connect(this->m_videoServer, SIGNAL(frameUpdated()), this, SLOT(OnFrameUpdated()));
	}

    this->loadAppParameters();
    this->initTrayIcon();
	this->syncMachine();
	QTimer::singleShot(APP_VIEWFINDER_PRESCALER, this, SLOT(OnViewfinderTimeout()));

#ifdef VEJAM_GUI_QT_TYPE
    //this->startServer();
    this->goAuthenticate();
#else
	//TODO: Migrar a => Qt WebEngine!!!
    QWebFrame* tempFrame = this->ui->webView->page()->mainFrame();
    QObject::connect(tempFrame, SIGNAL(javaScriptWindowObjectCleared()),this, SLOT(addJSObject()));
    QObject::connect(this->ui->webView, SIGNAL(loadStarted()),this, SLOT(loadStarted()));
    QObject::connect(this->ui->webView, SIGNAL(loadProgress(int)),this, SLOT(loadProgress(int)));
    QObject::connect(this->ui->webView, SIGNAL(loadFinished(bool)),this, SLOT(loadFinished(bool)));
	
	this->loadWebInterface();
    qDebug() << "MainWindow::MainWindow(END)";
#endif
}

void MainWindow::loadWebInterface()
{
 QString url = "http://";
    url += this->m_serverUrl;	

    if(this->m_autoStart)
    {
        url += "/app-gui-run.php?username=";
        url += this->m_username;
        //url += "&password=";
		url += "&hash=";
		url += QString(QCryptographicHash::hash(this->m_password.toUtf8(), QCryptographicHash::Md5).toHex());
		url += "&sourceId=";
		url += this->loadParam(QString("aplicacion"),QString("streamming-id"));
    }
    else
    {
        url += "/app-gui-welcome.html";
    }

    qDebug() << url;    
    this->ui->webView->load(QUrl(url));
}

MainWindow::~MainWindow()
{
    this->releaseAllocatedPointers();
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    case QEvent::WindowStateChange:
    {
        if(isMinimized())
        {
            this->hide();
			emit webImageStop();
        }
        else
        {
            this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);						
        }
    }
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "VEJAM",
                                                                tr("Seguro que quieres cerrar vejam.exe?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}

#ifdef VEJAM_GUI_WEBKIT_TYPE
void MainWindow::loadStarted()
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    this->p_ld = new loadingDialog(this);
    this->p_ld->show();
}

void MainWindow::loadProgress(int progress)
{
    ui->progressBar->setValue(progress);
    if(this->p_ld)
    {
        this->p_ld->setProgress(progress);
    }
}
#else
void MainWindow::loadStarted(){}
void MainWindow::loadProgress(int progress){}
#endif

void MainWindow::loadFinished(bool result)
{		
	this->p_ld->hide();
	if(this->p_ld)
	{
		delete this->p_ld;
		this->p_ld = 0;
	}

	if(result ==  false)
	{
		if(this->m_webInterfaceLoadTryouts > APP_WEB_INTERFACE_LOAD_TRYOUTS)
		{
			QMessageBox msgBox(this);
			QString msg = "No se puede establecer conexión con el servidor VEJAM.\r\n\r\n";
			msg += "Can't connect to VEJAM server.";
			msgBox.setText(msg);
			//msgBox.setWindowIcon(QIcon(QPixmap(":/png/img/vejam_toolbar_h48.png")));
			msgBox.exec();		
			this->m_webInterfaceLoadTryouts = 0;
			QTimer::singleShot(APP_RUN_RELOAD_PRESCALER, this, SLOT(reloadWebInterface()));
		}	
		else
		{
			QTimer::singleShot(APP_RUN_RELOAD_PRESCALER, this, SLOT(reloadWebInterface()));
			this->m_webInterfaceLoadTryouts++; 
		}				
	}
}

void MainWindow::reloadWebInterface()
{
	this->loadWebInterface();
}

#ifdef VEJAM_GUI_WEBKIT_TYPE
void MainWindow::addJSObject()
{
    this->ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("vejamApp"), this);
}
#else
void MainWindow::addJSObject(){}
#endif

void MainWindow::setDefaultParameters()
{
	this->saveParam(QString("aplicacion"),QString("username"),QString("user@name.here"));
	this->saveParam(QString("aplicacion"),QString("password"),QString("password"));
	this->saveParam(QString("aplicacion"),QString("sync-interval"),QString("60"));
    this->saveParam(QString("aplicacion"),QString("webkit-debug"),QString("0"));
    this->saveParam(QString("aplicacion"),QString("streamming-mode"),QString("2")); //1: WebKit, 2: MJPEG
	this->saveParam(QString("aplicacion"),QString("streamming-id"),QString("0")); //1...8
	this->saveParam(QString("aplicacion"),QString("streamming-alias"),QString("My Webcam!")); //1...8
	this->saveParam(QString("aplicacion"),QString("server-url"),QString("www.vejam.info/app-gui")); //http://www.vejam.info/app-gui/app-gui-welcome.html
    this->saveParam(QString("conexion"),QString("webkit-port"),QString("40001"));
    this->saveParam(QString("conexion"),QString("mjpeg-port"),QString("50001"));
	this->saveParam(QString("conexion"),QString("mjpeg-uri"),QString("/stream.html"));
    this->saveParam(QString("video"),QString("resolucion-x"),QString("320"));
    this->saveParam(QString("video"),QString("resolucion-y"),QString("240"));
    this->saveParam(QString("video"),QString("calidad"),QString("-1"));
    this->saveParam(QString("video"),QString("framerate-max"),QString("6"));    
	this->saveParam(QString("device"),QString("selected"),QString("1"));	//Indica la camara per defecte.	
    this->fileSave();
}

void MainWindow::releaseAllocatedPointers()
{
    //if(this->m_camera) delete this->m_camera;
    //if(this->m_imageCapture) delete this->m_imageCapture;
    //if(this->m_viewfinder) delete this->m_viewfinder;
}

QString MainWindow::getUrlDataImage()
{	
    QString data = "data:image/jpeg;base64,";
    data += this->m_videoServer->currentFrame2Base64Jpeg();
    return data;
}

void MainWindow::showApp(bool show)
{
    if(show) this->show();
    else this->hide();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "MainWindow::trayIconActivated( " << reason << " )";
    this->show();
    this->activateWindow();
    this->raise();
}

void MainWindow::initTrayIcon()
{
    this->m_trayIcon = new QSystemTrayIcon(this);
    connect(this->m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    this->setTrayIconState(trIndeterminado);   
}

void MainWindow::setTrayIconState(int newState)
{    
    switch(newState)
    {
        case trIdle: this->m_icon = QIcon(":/png/img/vejam_small_h22_white.png"); break;
        case trWaitingConnection: this->m_icon = QIcon(":/png/img/vejam_small_h22_yellow.png"); break;
        case trClientConnected: this->m_icon = QIcon(":/png/img/vejam_small_h22_green.png"); break;
        case trError: this->m_icon = QIcon(":/png/img/vejam_small_h22_red.png"); break;
        case trIndeterminado:
        default: this->m_icon = QIcon(":png/img/vejam_small_h22_blue.png"); break;
                break;
    };

    this->m_trayIcon->setIcon(this->m_icon);
    this->m_trayIcon->show();
}

 void MainWindow::startServer()
 {
     qint16 v;
     QString s;
     
     this->syncMachineSet(sstIdleSync);

	 this->loadAppParameters();

     switch(this->m_streammingMode)
     {
        case smodWebKit:
            if(this->m_websockServer) delete this->m_websockServer;
            this->m_websockServer = new  QtKWebsockServer(this);
            connect(this->m_websockServer, SIGNAL(socketReset()), this, SLOT(OnSocketReset()));
            connect(this->m_websockServer, SIGNAL(stateChanged(int)), this, SLOT(OnStateChanged(int)));
            connect(this->m_websockServer, SIGNAL(dataReceived(QByteArray)), this, SLOT(OnDataReceived(QByteArray)));
            connect(this->m_websockServer, SIGNAL(dataReceivedAjax(QByteArray)), this, SLOT(OnDataReceivedAjax(QByteArray)));
            v = this->m_appParameters->loadParam(QString("conexion"),QString("webkit-port"),0).toInt();
            this->m_websockServer->start(v);
            break;

        case smodMjpeg:
            break;

        default:
         break;
     }

     qDebug() << "startServer( m_streammingMode= " << this->m_streammingMode << " )";

	 v = this->m_appParameters->loadParam(QString("device"),QString("selected"),0).toInt();
	 s = this->m_appParameters->loadParam(QString("device"),QString("name"),v);
	 	 	 
     this->syncMachineSet(sstAskForIp);
 }

 void MainWindow::OnSocketReset()
 {
    
 }

 void MainWindow::OnStateChanged(int newState)
 {
     qDebug() << "OnStateChanged(" << newState  << ")";
     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
     QString msg;
     //MessageIcon { NoIcon, Information, Warning, Critical
     switch(newState)
     {
         case QtKWebsockServer::stUnknown:
              this->setTrayIconState(MainWindow::trIndeterminado);
              icon = QSystemTrayIcon::Warning;
              msg = "Error desconocido";
              break;

         case QtKWebsockServer::stListening:
             this->setTrayIconState(MainWindow::trWaitingConnection);
             icon = QSystemTrayIcon::Information;
             msg = "Funcionando, a la espera de conexiones...";
             break;

         case QtKWebsockServer::stHandShake:
              break;

         case QtKWebsockServer::stConnected:
             this->setTrayIconState(MainWindow::trClientConnected);
             icon = QSystemTrayIcon::Information;
			 msg = "Conexión remota establecida!";
             break;

         case QtKWebsockServer::stDisconnected:
             this->setTrayIconState(MainWindow::trIdle);
             icon = QSystemTrayIcon::Information;
			 msg = "Conexión remota finalizada!";
             break;

         case QtKWebsockServer::stError:
         default:
             this->setTrayIconState(MainWindow::trError);
             icon = QSystemTrayIcon::Critical;
             if(this->m_websockServer)
			 {
				msg = this->m_websockServer->getLastErrorDescription();
			 }
			 else
			 {			 
				msg = "Falta objeto m_websockServer?";
			 }
             break;
     }

	 if(msg.size()) this->m_trayIcon->showMessage(QString("VEJAM Info:"),msg,icon);
 }

void MainWindow::OnFrameUpdated()
{
     if(isMinimized() == false)
	 {         		
		emit webImageReady();
	 }
}

void MainWindow::OnViewfinderTimeout()
{
	 if(isMinimized() == false)
	 {         
		this->m_videoServer->Capture();
     }
	 QTimer::singleShot(APP_VIEWFINDER_PRESCALER, this, SLOT(OnViewfinderTimeout()));
}

void MainWindow::OnDataReceived(QByteArray data)
{        	
	QJsonDocument jsonDoc;
	QJsonObject json;
	
	jsonDoc = QJsonDocument::fromJson(data);

	qDebug() << "OnDataReceived(" << jsonDoc << ")";

	if(jsonDoc.isObject())
	{
		json = jsonDoc.object();

		if( json.value(QString("ack")).toString().compare("ok") == 0 )
		{
			//this->runMachineSet(stWaitState);
		}
		else if( json.value(QString("ack")).toString().compare("close") == 0 )
		{
			//this->runMachineSet(stRemoteCloseRequest);
		}
		
		//TODO: Remote settings setup...
	}
}

void MainWindow::OnDataReceivedAjax(QByteArray data)
{
    qDebug() << "OnDataReceivedAjax()";
}

void MainWindow::loadAppParameters()
{
    QString sData;
    qint16 data;  //ms
    data = this->loadParam(QString("video"),QString("framerate-max")).toInt();
    if(data)
    {
        if(data > 24) data = 24;
        this->m_frameInterval = ((1000 / data) / APP_RUN_TIMER_PRESCALER);
    }

    data = this->loadParam(QString("aplicacion"),QString("sync-interval")).toInt();
    if(data)
    {
        this->m_syncInterval = data;
    }
	
#ifdef VEJAM_GUI_WEBKIT_TYPE
    if(!this->loadParam(QString("aplicacion"),QString("webkit-debug")).compare("1"))
    {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    }
#endif
    this->m_serverUrl = this->loadParam(QString("aplicacion"),QString("server-url"));
    this->m_streammingMode = this->loadParam(QString("aplicacion"),QString("streamming-mode")).toInt();
	
    if(!this->loadParam(QString("aplicacion"),QString("auto-start")).compare("1"))
    {
        this->m_autoStart = true;
        this->m_username = this->loadParam(QString("aplicacion"),QString("username"));
        this->m_password = this->loadParam(QString("aplicacion"),QString("password"));
    }
    else this->m_autoStart = false;

	if(!this->loadParam(QString("aplicacion"),QString("file-log")).compare("1"))
	{
		qInstallMessageHandler(vejamLogger);
	}
}


void MainWindow::saveParam(QString groupName, QString paramName, QString paramValue, quint16 order)
{
    if(this->m_appParameters)
    {
        this->m_appParameters->saveParam(groupName, paramName,  paramValue, order);
        this->loadAppParameters();
    }
}

QString MainWindow::loadParam(QString groupName, QString paramName, quint16 order)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->loadParam(groupName, paramName, order);
    }

	return 0;
}

bool MainWindow::fileLoad(bool showAlerts)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->fileLoad(showAlerts);
    }

	return false;
}

bool MainWindow::fileSave()
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->fileSave();
    }

	return false;
}

QString MainWindow::getVersion()
{
	return QString(VEJAM_APP_VERSION);
}

bool MainWindow::wGetFile(QString url)
{
	if(this->p_manager) this->p_manager->deleteLater();

	this->p_manager = new QNetworkAccessManager(this);
	connect(this->p_manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(OnwGetfileDone(QNetworkReply*)));
 
	this->p_manager->get(QNetworkRequest(QUrl(url)));

	return true;
}

bool MainWindow::wSysExec(QString sysExec)
{
	this->m_sysExec = sysExec;
	return true;
}

void MainWindow::OnwGetfileDone(QNetworkReply* reply)
{
	QByteArray data;
	data = reply->readAll();

	if(data.size())
	{
		QFile file("updater.exe");
		file.open(QIODevice::WriteOnly);
		file.write(data);
		file.close();
		
		if(this->m_sysExec.size() > 0)
		{
			QProcess::execute(this->m_sysExec);
			this->m_sysExec.clear();
			//Lanzamos un proceso de instalación que sobrescribe este ejecutable asi que... tenemos que salir ;-)
			this->close();
		}
	}
}

void MainWindow::goAuthenticate()
{

}

void MainWindow::updateSyncRealm(QString newRealm)
{
	this->m_currentRealm = newRealm;
	qDebug() << "MainWindow::updateSyncRealm( " << this->m_currentRealm  << " )";
}

//Q:Uy! y esto?...
//A:Bueno partimos el archivo para que no sea tan grande.. y evitamos también algunos problemas raros de compilacion... no se.. yo soy asin...
#include "syncmachine.cpp"
