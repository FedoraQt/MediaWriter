#!/bin/bash

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

ROOTPATH=$(realpath "$SCRIPTDIR/../../")
BUILDPATH="$ROOTPATH/build"

MEDIAWRITER="$BUILDPATH/app/release/mediawriter.exe"
HELPER="$BUILDPATH/app/helper.exe"

PACKAGES="mingw32-mediawriter mingw32-qt5-qtbase mingw32-qt5-qtdeclarative mingw32-qt5-qtquickcontrols mingw32-nsis"

BINARIES="libstdc++-6.dll libwinpthread-1.dll libgcc_s_sjlj-1.dll libpng16-16.dll libharfbuzz-0.dll libpcre-1.dll libintl-8.dll iconv.dll libpcre16-0.dll libEGL.dll libglib-2.0-0.dll libGLESv2.dll zlib1.dll Qt5Core.dll Qt5Gui.dll Qt5Network.dll Qt5Qml.dll Qt5Quick.dll Qt5Svg.dll Qt5Widgets.dll Qt5WinExtras.dll"
PLUGINS="imageformats/qjpeg.dll imageformats/qsvg.dll platforms/qwindows.dll"
QMLMODULES="Qt QtQml QtQuick/Controls QtQuick/Dialogs QtQuick/Extras QtQuick/Layouts QtQuick/PrivateWidgets QtQuick/Window.2 QtQuick.2"

BIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_BINS)
PLUGIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PLUGINS)
QML_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_QML)

echo "=== Installing dependencies"
dnf install $PACKAGES

echo "=== Building"
mkdir -p "$BUILDPATH"
pushd "$BUILDPATH" >/dev/null
mingw32-qmake-qt5 ..
mingw32-make -j9 >/dev/null

pushd app/release >/dev/null

if [ ! -f "$MEDIAWRITER" ] || [ ! -f "$HELPER" ]; then
    echo "$MEDIAWRITER doesn't exist." 
    exit 1
fi

echo "=== Removing object and MOC files"
rm -f *.cpp
rm -f *.o

echo "=== Copying dlls"
for i in $BINARIES; do
    mkdir -p $(dirname $i)
    cp -r "${BIN_PREFIX}/${i}" "$(dirname $i)"
done

echo "=== Copying plugins"
for i in $PLUGINS; do
    mkdir -p $(dirname $i)
    cp -r "${PLUGIN_PREFIX}/${i}" "$(dirname $i)"
done

echo "=== Copying QML modules"
for i in $QMLMODULES; do
    mkdir -p $(dirname $i)
    cp -r "${QML_PREFIX}/${i}" "$(dirname $i)"
done

echo "=== Inserting helper"
cp "$HELPER" .

#echo "=== Compressing binaries"
#upx $(find . -name "*.exe")
#upx $(find . -name "*.dll")

#zip -r mediawriter.zip * >/dev/null

popd >/dev/null
popd >/dev/null

echo "=== Composing installer"
makensis "$SCRIPTDIR/mediawriter.nsi" >/dev/null

echo "=== Installer is located in $SCRIPTDIR/FMW-setup.exe"
