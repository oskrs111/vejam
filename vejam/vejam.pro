#-------------------------------------------------
#
# Project created by QtCreator 2013-12-07T18:45:37
#
#-------------------------------------------------

QT += core gui
QT += multimedia multimediawidgets
QT += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vejam
TEMPLATE = app


#LIBS += -lwinmm -lWs2_32 -lQt5PlatformSupport

SOURCES += main.cpp\
        mainwindow.cpp \
        qtkwebsockserver.cpp \
    qtkapplicationparameters.cpp \
    loadingdialog.cpp \
    syncmachine.cpp

HEADERS  += mainwindow.h \
    qtkwebsockserver.h \
    qtkapplicationparameters.h \
    loadingdialog.h

FORMS    += mainwindow.ui \
    loadingdialog.ui

RESOURCES += \
    vejam.qrc
