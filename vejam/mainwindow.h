#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCamera>
#include <QMainWindow>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QSystemTrayIcon>
#include <QImageEncoderSettings>
#include <QtNetwork>
#include "loadingdialog.h"
#include "qtkwebsockserver.h"
#include "qtkapplicationparameters.h"
#include "qtkcapturebuffer.h"

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
    QCamera* m_camera;
    QCameraImageCapture* m_imageCapture;
    //QCameraViewfinder* m_viewfinder;
    QtKCaptureBuffer*  m_viewfinder;
    //QAbstractVideoSurface* m_viewfinder;
    QSystemTrayIcon* m_trayIcon;
    QtKWebsockServer* m_websockServer;    
    QIcon m_icon;
    QImageEncoderSettings m_encodeSettings;

    bool m_imageReady;
    bool m_autoStart;

    int m_state;
    int m_syncState;
    int m_frameInterval;
    int m_syncInterval;
	int m_streammingMode;
    
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
    QList <struct vjCameraDevice> m_devices;

    bool loadAvaliableCameras();
    void runMachineSet(int newState);    
    void setCamera(const QByteArray &cameraDevice);
    void image2Base64();
    QByteArray image2ByteArray();
    void releaseAllocatedPointers();
    void initTrayIcon();
    void setTrayIconState(int newState);
    void changeEvent(QEvent *e);
	void closeEvent (QCloseEvent *event);
    void setDefaultParameters();
    void loadAppParameters();

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
        stWaitState
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
    void runMachine();
    void syncMachine();
    void addJSObject();
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool result);
    void processCapturedImage(int id, QImage image);
    void displayCaptureError(int id,QCameraImageCapture::Error error, QString errorString);
    void updateCameraState(QCamera::State state);
    void displayCameraError(QCamera::Error error);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void syncAckReply(QNetworkReply* reply);
    void askForIpReply(QNetworkReply* reply);

    void OnSocketReset();
    void OnStateChanged(int newState);
    void OnDataReceived(QByteArray data);
    void OnDataReceivedAjax(QByteArray data);
	void OnwGetfileDone(QNetworkReply* reply);


signals:
    void webImageReady();

};

struct vjCameraDevice
{
    QByteArray m_name;
    QString m_description;
};

#endif // MAINWINDOW_H
