TEMPLATE = lib

CONFIG += plugin qmltypes qt

TARGET = adwaitathemeplugin

QT += qml quick quickcontrols2

CONFIG += c++11

HEADERS += adwaitathemeplugin.h \
    theme.h \
    units.h

SOURCES += adwaitathemeplugin.cpp \
    theme.cpp \
    units.cpp

QML_IMPORT_NAME = org.fedoraproject.AdwaitaTheme
QML_IMPORT_MAJOR_VERSION = 2

lupdate_only {
    SOURCES += $$PWD/*.qml \
        $$PWD/private/*.qml \
        $$PWD/*.cpp
    HEADERS += $$PWD/*.h
}

qml.files += $$PWD/*.qml
qml.path = $$[QT_INSTALL_QML]/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme

qmlprivate.files += $$PWD/private/*.qml
qmlprivate.path =  $$[QT_INSTALL_QML]/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/private

qmldir.files += qmldir
qmldir.path = $$[QT_INSTALL_QML]/org/fedoraproject/AdwaitaTheme

target.path = $$[QT_INSTALL_QML]/org/fedoraproject/AdwaitaTheme

INSTALLS += target qml qmlprivate qmldir
