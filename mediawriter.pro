TEMPLATE = app

QT += qml quick widgets

CONFIG += c++11
CONFIG += console

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp

RESOURCES += qml.qrc \
    assets.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h
