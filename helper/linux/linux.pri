INCLUDEPATH += $$top_srcdir/helper/linux/

QT += dbus

CONFIG += link_pkgconfig
PKGCONFIG += liblzma

SOURCES += linux/drive.cpp
HEADERS += linux/drive.h
