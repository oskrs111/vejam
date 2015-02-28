#-------------------------------------------------
#
# Project created by QtCreator 2015-02-20T14:48:14
#
#-------------------------------------------------

QT       += core

QT       -= gui

#OpenSSL Libraries in Windows
INCLUDEPATH += "C:/OpenSSL-Win32/include"
LIBS += -L"C:\OpenSSL-Win32\lib\VC" -llibeay32MD
#OpenSSL Libraries in Windows - END


TARGET = vejamCryp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    vejamcrypt.cpp

HEADERS += \
    vejamcrypt.h


