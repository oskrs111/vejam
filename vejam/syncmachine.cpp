#include <QJsonObject>
#include "mainwindow.h"

void MainWindow::syncMachineSet(int newState)
{   
   qDebug() << "MainWindow::syncMachineSet(" << newState  << ")";
   this->m_syncState = newState;
}

void MainWindow::syncMachine()
//D'aqui ha de sortir la informació encriptada així que primer di demanem al php desde quina adreça ip estem comunicant
//i sincronizem despres... Total no hi ha pressa no?
{
    static int syncTime = 0;
    static QNetworkAccessManager *manager = 0;
    QString url = "";

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
            url += this->m_serverUrl;
            url += "/app-user-sync.php?vejamAskIp=1";
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(askForIpReply(QNetworkReply*)));
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
            url += this->m_serverUrl;
            url += "/app-user-sync.php?vejamSync=1";
            url += "&userName=";
            url += this->loadParam(QString("aplicacion"),QString("username"));            
			url += "&sourceId=";
            url += this->loadParam(QString("aplicacion"),QString("streamming-id"));
            url += "&syncData=";
            url += this->getSyncString();   
			url += "&syncEncript=1";				
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(syncAckReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            qDebug() << "MainWindow::syncMachine(sstAskForIp:" << url << ")";
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

        case sstSyncError:
             this->syncMachineSet(sstGoSync);
             break;

        default: break;
    }

    QTimer::singleShot(APP_RUN_SYNC_PRESCALER, this, SLOT(syncMachine()));
    syncTime += 1;
    QThread::msleep(1);
}

QString MainWindow::getSyncString()
//JSON Base64 encoded string.
{    
    QJsonObject json;
    QString enc64;


    json.insert("server-ip",QJsonValue(this->m_lastIpReply));
    json.insert("webkit-port",QJsonValue(this->loadParam(QString("conexion"),QString("webkit-port"))));
    json.insert("mjpeg-port",QJsonValue(this->loadParam(QString("conexion"),QString("mjpeg-port"))));
    json.insert("streamming-mode",QJsonValue(this->loadParam(QString("aplicacion"),QString("streamming-mode"))));

    QJsonDocument jsonDoc(json);
    
    enc64 = this->getEncryptedString(QString(jsonDoc.toJson(QJsonDocument::Compact)), this->loadParam(QString("aplicacion"),QString("password")));
    //enc64 = this->getEncryptedString(QString("hola"), this->m_password);    
	enc64.remove("\r");		//Pol que tiene que pasar esto...?
	enc64.remove("\n");		//Pol que tiene que pasar esto...?
	enc64.replace("+","%2B"); //URL-encoded, para poder utilizarlo como parametro del GET!
    return enc64;
}

#include <QVariant>
QString MainWindow::getEncryptedString(QString cleanText, QString password)
//...Bueno vale es un poco raruzo hacerlo asin pero esto me asegura 100% de compatibilidad con la decodificación
//en el lado del cliente... Cuando encuentre como hacelo con OpenSSL + Alguna libreria JAVASCRIPT lo hare!
{
	QVariant result;
	QString jeval("doEncrypt('");
	jeval += cleanText;
	jeval += "','";
	jeval += password;
	jeval += "')";

	result = this->ui->webView->page()->mainFrame()->evaluateJavaScript(jeval); 
	return result.toString();	
	//return result.toByteArray();
}


/*
#include <openssl/aes.h>
QByteArray MainWindow::getEncryptedString(QString cleanText, QString password)
//http://www.essentialunix.org/index.php?option=com_content&view=article&id=48:qcatutorial&catid=34:qttutorials&Itemid=53
//https://github.com/JPNaude/dev_notes/wiki/Using-the-Qt-Cryptographic-Architecture-with-Qt5
//http://stackoverflow.com/questions/14681012/how-to-include-openssl-in-a-qt-project
//http://stackoverflow.com/questions/9889492/how-to-do-encryption-using-aes-in-openssl
{
    QByteArray enc_out;
    QByteArray enc_res;
    unsigned char* key = 0;
    unsigned char* data = 0;
    unsigned char* out = 0;
    unsigned char* res = 0;
    unsigned char iv[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    enc_out.resize(cleanText.size() + (2*AES_BLOCK_SIZE));
    enc_out.fill(0x00);

    enc_res.resize(cleanText.size() + (2*AES_BLOCK_SIZE));
    enc_res.fill(0x00);


    key  = (unsigned char*)password.data();
    data = (unsigned char*)cleanText.data();
    out  = (unsigned char*)enc_out.data();
    res  = (unsigned char*)enc_res.data();


    AES_KEY enc_key;
    AES_KEY dec_key;
    AES_set_encrypt_key(key, 128, &enc_key);
	AES_encrypt(data, out, &enc_key);
	//AES_cbc_encrypt(data, out, password.size(), &enc_key, iv, 0);

    AES_set_decrypt_key(key, 128, &dec_key);
    AES_decrypt(out, res, &dec_key);

    return enc_out.toBase64();
}
*/

void MainWindow::askForIpReply(QNetworkReply* reply)
{
     if (reply->error() == QNetworkReply::NoError)
     {
          QByteArray bytes = reply->readAll();  // bytes
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

void MainWindow::syncAckReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
         QByteArray bytes = reply->readAll();  // bytes
		 qDebug() << "MainWindow::syncAckReply(reply:" << bytes << ")";
         //QJsonDocument jsonReply;
         //jsonReply.fromBinaryData(bytes,QJsonDocument::BypassValidation);
         //this->m_lastIpReply = jsonReply.object().value("remoteIp").toString();
         this->syncMachineSet(sstWaitSyncAckDone);
     }
     else
     {
         this->syncMachineSet(sstSyncError);
     }

     reply->deleteLater();
}

