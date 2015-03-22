#ifndef VEJAM_NO_GUI
#include <QMessageBox>
#endif
#include <QTimer.h>
#include <QVariant>
#include "syncMachine.h"

syncMachine::syncMachine(QtKApplicationParameters *params)
{
    this->m_appParameters = params;
	this->syncMachineSet(sstIdleSync);
	this->m_syncInterval = this->loadParam(QString("aplicacion"),QString("sync-interval")).toInt();
	QTimer::singleShot(APP_RUN_SYNC_PRESCALER, this, SLOT(OnSyncMachineRun()));
}

void syncMachine::syncMachineSet(int newState)
{   
   qDebug() << "syncMachine::syncMachineSet(" << newState  << ")";
   this->m_syncState = newState;
}

void syncMachine::OnSyncMachineRun()
//D'aqui ha de sortir la informació encriptada així que primer di demanem al php desde quina adreça ip estem comunicant
//i sincronizem despres... Total no hi ha pressa no?
{
    static int syncTime = 0;
    static QNetworkAccessManager *manager = 0;
    QString url = "";

#ifndef VEJAM_NO_GUI
    QMessageBox msgBox(this);
#endif
	QString msg;

    switch(this->m_syncState)
    {
        case sstIdleSync: break;

        case sstGoSync:
             syncTime = 0;
             this->syncMachineSet(sstSyncWait);
             break;

        case sstSyncWait:
            if(syncTime > this->m_syncInterval)
            {
                this->syncMachineSet(sstAskForIp);
            }
            break;

        case sstAskForIp:
            url = "http://";
            url += this->loadParam(QString("aplicacion"),QString("server-url"));
            url += "/app-user-sync.php?vejamAskIp=1";
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(OnAskForIpReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            this->syncMachineSet(sstWaitForIp);
            qDebug() << "MainWindow::syncMachine(sstAskForIp:" << url << ")";
            break;

        case sstWaitForIp:
            //Esperem el callback desde askForIpReply(QNetworkReply*)...
            break;

        case sstWaitForIpDone:
            this->syncMachineSet(sstSendSync);
            if(manager)
            {
                delete manager;
                manager = 0;
            }
            break;

        case sstSendSync:
            url = "http://";
            url += this->loadParam(QString("aplicacion"),QString("server-url"));
            url += "/app-user-sync.php?vejamSync=1";
            url += "&userName=";
            url += this->loadParam(QString("aplicacion"),QString("username"));            
			url += "&sourceId=";
            url += this->loadParam(QString("aplicacion"),QString("streamming-id"));
            url += "&cleanupEnable=";
            url += this->loadParam(QString("aplicacion"),QString("cleanup-enable"));
            url += "&syncData=";
            url += this->getSyncString();   
			//OSLL: 0 = No Encrypt, 1 = AES, 2 = DES.
			url += "&syncEncript=2";
			url += "&syncRealm=";
			url += this->m_currentRealm;
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(OnSyncAckReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            qDebug() << "syncMachineRun(sstSendSync:" << url << ")";
            this->syncMachineSet(sstWaitSyncAck);
            break;

        case sstWaitSyncAck:
             //Esperem el callback desde syncAckReply(QNetworkReply*)...
             break;

        case sstWaitSyncAckDone:
            this->syncMachineSet(sstGoSync);
            if(manager)
            {
                delete manager;
                manager = 0;
            }
            break;

		case sstSyncErrorWrongRealm:	
			 msg += "Ha ocurrido un error de sincronización.\r\nPor favor reinicie la aplicación vejam para solucionar el probelema.\r\n\r\n";              
			 msg += "A syncronization error has occurred.\r\nPlease restart vejam application to solve the problem.";
#ifndef VEJAM_NO_GUI
				msgBox.setText(msg);				
				msgBox.exec();
#else
                qDebug() << msg;
#endif

		case sstSyncError:
             this->syncMachineSet(sstGoSync);
             break;

        default: break;
    }

    QTimer::singleShot(APP_RUN_SYNC_PRESCALER, this, SLOT(OnSyncMachineRun()));
    syncTime += 1;
    QThread::msleep(1);
}

QString syncMachine::getSyncString()
//JSON Base64 encoded string.
{    
    QJsonObject json;
    QString enc64;
	
	
    json.insert("server-ip",QJsonValue(this->m_lastIpReply));
    json.insert("webkit-port",QJsonValue(this->loadParam(QString("conexion"),QString("webkit-port"))));
    json.insert("mjpeg-port",QJsonValue(this->loadParam(QString("conexion"),QString("mjpeg-port"))));
	json.insert("mjpeg-uri",QJsonValue(this->loadParam(QString("conexion"),QString("mjpeg-uri"))));
    json.insert("streamming-mode",QJsonValue(this->loadParam(QString("aplicacion"),QString("streamming-mode"))));
	json.insert("streamming-alias",QJsonValue(this->loadParam(QString("aplicacion"),QString("streamming-alias"))));
	
    QJsonDocument jsonDoc(json);
    
    enc64 = this->getEncryptedString(QString(jsonDoc.toJson(QJsonDocument::Compact)), this->loadParam(QString("aplicacion"),QString("password")));
    //QString __enc64;
	//__enc64 = this->getEncryptedStringOpenSSL(QString(jsonDoc.toJson(QJsonDocument::Compact)), this->loadParam(QString("aplicacion"),QString("password")));  
	enc64.remove("\r");		//Pol que tiene que pasar esto...?
	enc64.remove("\n");		//Pol que tiene que pasar esto...?
	enc64.replace("+","%2B"); //URL-encoded, para poder utilizarlo como parametro del GET!
    qDebug() << "syncMachine::getSyncString(" << enc64 << ")";
	return enc64;
}



QString syncMachine::getEncryptedString(QString cleanText, QString password)
{
    //QByteArray ret = cleanText.toLatin1();
    //return QString(ret.toBase64());

	m_CryptMachine.vcSetEncryptMode(vejamCrypt::vcDES_ECB);
	return m_CryptMachine.vcEncryptB64(cleanText,password);
}



void syncMachine::OnAskForIpReply(QNetworkReply* reply)
{
     if (reply->error() == QNetworkReply::NoError)
     {
          QByteArray bytes = reply->readAll();  // bytes
		  qDebug() << "syncMachine::askForIpReply(reply:" << bytes << ")";
          QJsonDocument jsonReply;
		  QJsonParseError jsonError;

          jsonReply = QJsonDocument::fromJson(bytes,&jsonError);
          this->m_lastIpReply = jsonReply.object().value("remoteIp").toString();
          this->syncMachineSet(sstWaitForIpDone);
      }
      else
      {
          this->syncMachineSet(sstSyncError);
      }

      reply->deleteLater();
}

void syncMachine::OnSyncAckReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
         QByteArray bytes = reply->readAll();  // bytes
		 qDebug() << "syncMachine::syncAckReply(reply:" << bytes << ")";
         
		QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes);
        QJsonObject jsonObject = jsonResponse.object();
		QString result = jsonObject["result"].toString();
		if(result.compare("success"))
		{
			this->syncMachineSet(sstSyncErrorWrongRealm);	
			return;
		}

		this->updateSyncRealm(jsonObject["syncRealm"].toString()); 
        this->syncMachineSet(sstWaitSyncAckDone);
     }
     else
     {
         this->syncMachineSet(sstSyncError);
     }

     reply->deleteLater();
}

void syncMachine::OnAuthDone(QString syncRealm)
{
    this->updateSyncRealm(syncRealm);
    this->syncMachineSet(sstAskForIp);

	qDebug() << "Iniciando sincronizacion con el servicio VEJAM...";
}

void syncMachine::updateSyncRealm(QString realm)
{
    this->m_currentRealm = realm;
}

QString syncMachine::loadParam(QString groupName, QString paramName)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->loadParam(groupName, paramName, 0);
    }

    return 0;
}

