#-------------------------------------------------
#
# Project created by QtCreator 2015-02-14T14:05:26
#
#-------------------------------------------------

QT       += core network
QT       -= gui

#OpenSSL Libraries in Windows
INCLUDEPATH += "C:/OpenSSL-Win32/include"
LIBS += -L"C:\OpenSSL-Win32\lib\VC" -llibeay32MD
#OpenSSL Libraries in Windows - END


INCLUDEPATH += "./../../vejam"  #=> "vejam/qtkapplicationparameters.h"
INCLUDEPATH += "./../../vejamCrypt/vejamCrypt"  #=> "vejamCrypt/vejamCryp/vejamcrypt.h"

DEFINES += VEJAM_NO_GUI

TARGET = vejamSync
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../../vejam/qtkapplicationparameters.cpp \
    ../../vejamCrypt/vejamCrypt/vejamcrypt.cpp \
    authmachine.cpp \
    syncmachine.cpp

HEADERS += \
    ../../vejamCrypt/vejamCrypt/vejamcrypt.h \
    authMachine.h \
    syncMachine.h \
    ../../vejam/qtkapplicationparameters.h \
    main.h
