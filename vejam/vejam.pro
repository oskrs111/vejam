#-------------------------------------------------
#
# Project created by QtCreator 2013-12-07T18:45:37
#
#-------------------------------------------------

QT += core gui
QT += multimedia multimediawidgets
QT += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#OpenSSL Libraries in Windows
INCLUDEPATH += "C:/OpenSSL-Win32/include"
#LIBS += -L"C:\OpenSSL-Win32\lib\VC" -llibeay32MD.lib
#OpenSSL Libraries in Windows - END

DEFINES += VEJAM_GUI_WEBKIT_TYPE
TARGET = vejam
TEMPLATE = app


#LIBS += -lwinmm -lWs2_32 -lQt5PlatformSupport

SOURCES += main.cpp\
        mainwindow.cpp \
        qtkwebsockserver.cpp \
    qtkapplicationparameters.cpp \
    loadingdialog.cpp \
    syncmachine.cpp \
    qtkcapturebuffer.cpp \
    qtkvideoserver.cpp \
    qtkHttpServer.cpp \
    qtkMjpgStreamer.cpp

HEADERS  += mainwindow.h \
    qtkwebsockserver.h \
    qtkapplicationparameters.h \
    loadingdialog.h \
    qtkcapturebuffer.h \
    qkHttpServer.h \
    qtkvideoserver.h \
    qtkMjpgStreamer.h \
    qtkHttpServer.h

FORMS    += mainwindow.ui \
    loadingdialog.ui

RESOURCES += \
    vejam.qrc
