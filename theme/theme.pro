TEMPLATE = lib

CONFIG += link_pkgconfig
PKGCONFIG += adwaita-qt

TARGET = adwaitathemeplugin

QT += qml quick quickcontrols2 widgets

CONFIG += c++11

HEADERS += adwaitathemeplugin.h \
    icon.h \
    theme.h

SOURCES += adwaitathemeplugin.cpp \
    icon.cpp \
    theme.cpp

QML_IMPORT_NAME = org.fedoraproject.AdwaitaTheme
QML_IMPORT_MAJOR_VERSION = 2

lupdate_only {
    SOURCES += $$PWD/qml/*.qml \
        $$PWD/qml/private/*.qml \
        $$PWD/*.cpp
    HEADERS += $$PWD/*.h
}

qml.files += $$PWD/qml/*.qml
qml.path = $$[QT_INSTALL_QML]/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme

qmlprivate.files += $$PWD/qml/private/*.qml
qmlprivate.path =  $$[QT_INSTALL_QML]/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/private

qmldir.files += qmldir
qmldir.path = $$[QT_INSTALL_QML]/org/fedoraproject/AdwaitaTheme

target.path = $$[QT_INSTALL_QML]/org/fedoraproject/AdwaitaTheme

INSTALLS += target qml qmlprivate qmldir
