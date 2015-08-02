#-------------------------------------------------
#
# Project created by QtCreator 2013-12-07T18:45:37
#
#-------------------------------------------------
DEFINES += VEJAM_GUI_WEBKIT_TYPE
TARGET = vejam
TEMPLATE = app

QT += core gui
QT += multimedia multimediawidgets
QT += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32
{
    #OpenSSL Libraries in Windows
    INCLUDEPATH += "C:/OpenSSL-Win32/include"
    LIBS += -L"C:/OpenSSL-Win32/lib/VC" -llibeay32MDd
}

linux*
{
    LIBS += -lwinmm -lWs2_32 -lQt5PlatformSupport
}

INCLUDEPATH += ./qtkApplicationParameters
INCLUDEPATH += ./qtkVideoServer
INCLUDEPATH += ./qtkHttpServer

SOURCES += main.cpp\
        mainwindow.cpp \
        qtkwebsockserver.cpp \
    qtkApplicationParameters/qtkapplicationparameters.cpp \
    loadingdialog.cpp \
    syncmachine.cpp \
    qtkVideoServer/qtkCaptureBuffer.cpp \
    qtkVideoServer/qtkVideoServer.cpp \
    qtkHttpServer/qtkHttpServer.cpp \
    qtkHttpServer/qtkMjpgStreamer.cpp

HEADERS  += mainwindow.h \
    qtkwebsockserver.h \
    qtkApplicationParameters/qtkapplicationparameters.h \
    loadingdialog.h \
    qtkVideoServer/qtkCaptureBuffer.h \
    qtkVideoServer/qtkVideoServer.h \
    qtkHttpServer/qtkMjpgStreamer.h \
    qtkHttpServer/qtkHttpServer.h

FORMS    += mainwindow.ui \
    loadingdialog.ui

RESOURCES += \
    vejam.qrc
