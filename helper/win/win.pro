TEMPLATE = app

QT += core network

CONFIG += c++11
CONFIG += console

TARGET = helper
DESTDIR = ../../app

include($$top_srcdir/deployment.pri)

target.path = $$LIBEXECDIR
INSTALLS += target

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h

DISTFILES += \
    helper.exe.manifest

QMAKE_MANIFEST = $${PWD}/helper.exe.manifest
