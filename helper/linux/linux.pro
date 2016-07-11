TEMPLATE = app

QT += core network dbus

CONFIG += c++11
CONFIG += console

TARGET = helper

include(deployment.pri)

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h
