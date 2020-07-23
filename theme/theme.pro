TEMPLATE = lib

CONFIG += staticlib plugin qmltypes qt

TARGET = adwaitathemeplugin

QT += qml quick quickcontrols2

CONFIG += c++11

HEADERS += adwaitathemeplugin.h \
    theme.h \
    units.h

SOURCES += adwaitathemeplugin.cpp \
    theme.cpp \
    units.cpp

RESOURCES += adwaitatheme.qrc

QML_IMPORT_NAME = AdwaitaTheme
QML_IMPORT_MAJOR_VERSION = 2

lupdate_only {
    SOURCES += $$PWD/*.qml \
        $$PWD/private/*.qml \
        $$PWD/*.cpp
    HEADERS += $$PWD/*.h
}
