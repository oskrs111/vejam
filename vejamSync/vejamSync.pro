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

#OpenSSL Libraries in raspi-linux
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/ssl/lib" -lcrypto
#OpenSSL Libraries in raspi-linux - END

#Qt Libraries in raspi-linux
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/QtPi5.4.2/plugins/bearer" -lqconnmanbearer
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/QtPi5.4.2/plugins/bearer" -lqgenericbearer
#Qt Libraries in raspi-linux - END

INCLUDEPATH += "./../vejam"  #=> "vejam/qtkapplicationparameters.h"
INCLUDEPATH += "./../vejamCrypt"  #=> "vejamCrypt/vejamCryp/vejamcrypt.h"

DEFINES += VEJAM_NO_GUI

TARGET = vejamSync
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../vejam/qtkapplicationparameters.cpp \
    ../vejamCrypt/vejamcrypt.cpp \
    authmachine.cpp \
    syncmachine.cpp

HEADERS += \
    ../vejamCrypt/vejamcrypt.h \
    authMachine.h \
    syncMachine.h \
    ../vejam/qtkapplicationparameters.h \
    main.h
