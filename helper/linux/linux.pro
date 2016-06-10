TEMPLATE = app

QT += core network dbus

CONFIG += c++11
CONFIG += console

TARGET = helper
DESTDIR = ../../app

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h
