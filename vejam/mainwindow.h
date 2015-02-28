#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtNetwork>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include "loadingdialog.h"
#include "QtkHttpServer.h"
#include "qtkvideoserver.h"
#include "qtkwebsockserver.h"
#include "qtkapplicationparameters.h"

#ifdef VEJAM_GUI_QT_TYPE
namespace Ui {
class MainWindowQt;
}
#else
namespace Ui {
class MainWindow;
}
#endif

#define APP_RUN_TIMER_PRESCALER 5  //ms
#define APP_RUN_SYNC_PRESCALER 1000 //ms
#define APP_VIEWFINDER_PRESCALER 100 //ms
#define APP_RUN_RELOAD_PRESCALER 500 //ms
#define APP_WEB_INTERFACE_LOAD_TRYOUTS 25

void vejamLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Q_INVOKABLE QString getUrlDataImage();
    Q_INVOKABLE void showApp(bool show);
    Q_INVOKABLE void startServer();
    Q_INVOKABLE void saveParam(QString groupName, QString paramName, QString paramValue, quint16 order = 0);
    Q_INVOKABLE QString loadParam(QString groupName, QString paramName, quint16 order = 0);
    Q_INVOKABLE bool fileLoad(bool showAlerts);
    Q_INVOKABLE bool fileSave();
	Q_INVOKABLE QString getVersion();
	Q_INVOKABLE void updateSyncRealm(QString newRealm);
	Q_INVOKABLE bool wGetFile(QString url);
	Q_INVOKABLE bool wSysExec(QString sysExec);
	
private:
    QtKApplicationParameters* m_appParameters;          
    QSystemTrayIcon* m_trayIcon;
    QtKWebsockServer* m_websockServer;    
    QIcon m_icon;
    QtkVideoServer* m_videoServer;
    QtkHttpServer*  m_httpServer;

/*
    QCamera* m_camera;
    QCameraImageCapture* m_imageCapture;
    QtKCaptureBuffer*  m_viewfinder;
    QImageEncoderSettings m_encodeSettings;
*/
    bool m_imageReady;
    bool m_autoStart;

    int m_state;
    int m_syncState;
    int m_frameInterval;
    int m_syncInterval;
	int m_streammingMode;
	int m_webInterfaceLoadTryouts;
    
    QString m_serverUrl;
    QString m_username;
    QString m_password;
    QString m_lastIpReply;
	QString m_currentRealm;

    QImage m_currentFrame;
    QByteArray m_currentBase64Frame;

	QNetworkAccessManager *p_manager;
	QString m_sysExec;

#ifdef VEJAM_GUI_QT_TYPE
    Ui::MainWindowQt *ui;
#else
    Ui::MainWindow *ui;
#endif
    loadingDialog* p_ld;    

    void runMachineSet(int newState);
    void releaseAllocatedPointers();
    void initTrayIcon();
    void setTrayIconState(int newState);
    void changeEvent(QEvent *e);
	void closeEvent (QCloseEvent *event);
    void setDefaultParameters();
    void loadAppParameters();
	void loadWebInterface();

    void goAuthenticate();
	
	//syncmachine.cpp!
	void syncMachineSet(int newState);
	QString getSyncString();
	QString getEncryptedString(QString cleanText, QString password);
	//QString getEncryptedStringOpenSSL(QString cleanText, QString password);
	
    enum runStates
    {
        stIdle = 0,
        stCapture,
        stWaitCapture,
        stBaseEncode,
        stSend,
        stWaitAck,
        stWaitAckDone,
        stWaitState,
		stRemoteCloseRequest,
		stEnd
    };

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


    enum trayIconStates
    {
        trIndeterminado = 0,
        trIdle,
        trWaitingConnection,
        trClientConnected,
        trError
    };

    enum streammingModes
    {
        smodNone = 0,
        smodWebKit,
        smodMjpeg
    };

public slots:    
    void syncMachine();
    void addJSObject();
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool result);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void syncAckReply(QNetworkReply* reply);
    void askForIpReply(QNetworkReply* reply);
	void reloadWebInterface();

    void OnSocketReset();
    void OnStateChanged(int newState);
    void OnDataReceived(QByteArray data);
    void OnDataReceivedAjax(QByteArray data);
	void OnwGetfileDone(QNetworkReply* reply);
	void OnFrameUpdated();
	void OnViewfinderTimeout();


signals:
    void webImageReady();
	void webImageStop();
};
#endif // MAINWINDOW_H
