TEMPLATE = lib

CONFIG += staticlib

DESTDIR = ../

HEADERS += libcheckisomd5.h \
        md5.h

SOURCES += libcheckisomd5.c \
        md5.c

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
