TEMPLATE = app

QT += core network dbus

CONFIG += c++11
CONFIG += console

TARGET = helper

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h
