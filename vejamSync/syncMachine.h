#ifndef __SYNC_MACHINE__
#define __SYNC_MACHINE__
#include <QtNetwork>
#include "vejamcrypt.h"
#include "qtkapplicationparameters.h"

#define APP_RUN_SYNC_PRESCALER 1000 //ms

class syncMachine: public QObject
{
    Q_OBJECT
public:
    syncMachine(QtKApplicationParameters *params);
	void syncMachineSet(int newState);
	void syncMachineRun();
	QString getSyncString();	
	QString getEncryptedString(QString cleanText, QString password);
	void askForIpReply(QNetworkReply* reply);    
    void updateSyncRealm(QString realm);

    enum syncStates
    {
        sstIdleSync = 0,
        sstGoSync,
        sstSyncWait,
        sstAskForIp,
        sstWaitForIp,
        sstWaitForIpDone,
        sstSendSync,
        sstWaitSyncAck,
        sstWaitSyncAckDone,
        sstSyncErrorWrongRealm,
        sstSyncError
    };

private:
    QString loadParam(QString groupName, QString paramName);
    vejamCrypt m_CryptMachine;
    int m_syncState;
    int m_syncInterval;
    QString m_currentRealm;
    QString m_lastIpReply;
    QtKApplicationParameters* m_appParameters;
	
public slots:	
    void OnSyncAckReply(QNetworkReply* reply);
	void OnAskForIpReply(QNetworkReply* reply);
    void OnAuthDone(QString syncRealm);
	void OnSyncMachineRun();
};

#endif
