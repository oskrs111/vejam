#ifndef __AUTH_MACHINE__
#define __AUTH_MACHINE__
#include <QtNetwork>
#include "qtkapplicationparameters.h"

#define APP_RUN_TIMER_PRESCALER 5  //ms
#define APP_RUN_AUTH_PRESCALER 1000 //ms
#define APP_RUN_RELOAD_PRESCALER 500 //ms

class authMachine: public QObject
{
    Q_OBJECT    
public:         
    authMachine(QtKApplicationParameters* params);
    void authMachineSetState(int newState);
    int authMachineGetState();
    void authMachineRun();
    QString getSyncRealm();

    enum authStates
    {
        sstIdleAuth = 0,
        sstDoneAuth,
        sstGoAuth,
        sstAuthWait,
        sstSendAuth,
        sstWaitAuthAck,
        sstWaitAuthAckDone,
        sstAuthError
    };

private:
    QString loadParam(QString groupName, QString paramName);
    int m_authState;
    int m_authInterval;
    QString m_currentRealm;
    QtKApplicationParameters* m_appParameters;

public slots:
    void OnAuthAckReply(QNetworkReply* reply);
	void OnAuthMachineRun();

signals:
    void authDone(QString syncRealm);

};

#endif
