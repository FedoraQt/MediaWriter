INCLUDEPATH += $$top_srcdir/helper/win/

LIBS += -lliblzma

SOURCES += win/drive.cpp
HEADERS += win/drive.h

DISTFILES += helper.exe.manifest

QMAKE_MANIFEST = $${PWD}/helper.exe.manifest
