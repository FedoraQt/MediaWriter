TEMPLATE = app

QT += core network dbus

CONFIG += c++11
CONFIG += console

TARGET = helper

include($$top_srcdir/deployment.pri)

target.path = $$LIBEXECDIR
INSTALLS += target

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h
