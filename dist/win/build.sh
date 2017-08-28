#!/bin/bash


# Usage
# ./build.sh - installs all the necessary packages and composes an installer (FMW-setup.exe)
# ./build.sh local - builds the mediawriter.exe binary itself and then composes the installer
# ./build.sh install - just installs the necessary packages
#
# The script will try to sign all binaries using your code signing certificate.
# You can provide the path to it using the $CERTPATH variable, the file then
# has to be named authenticode.pfx
# You have to provide the $CERTPASS with the path to a file containing the passphrase to your
# certificate (beware of the trailing last new line symbol)


pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

ROOTPATH=$(realpath "$SCRIPTDIR/../../")
BUILDPATH="$ROOTPATH/build"

MEDIAWRITER="$BUILDPATH/app/release/mediawriter.exe"
HELPER="$BUILDPATH/app/helper.exe"

if [ -z "$CERTPATH" ]; then
    CERTPATH=~
fi
if [ -z "$CERTPASS" ]; then
    CERTPASS="$CERTPATH/authenticode.pass"
fi

PACKAGES="mingw32-qt5-qmake mingw32-mediawriter mingw32-qt5-qtbase mingw32-qt5-qtdeclarative mingw32-qt5-qtquickcontrols mingw32-qt5-qtwinextras mingw32-xz-libs mingw32-nsis mono-devel"

echo "=== Installing dependencies"
dnf install $PACKAGES

if [ "$1" == "install" ]; then
    exit 0
fi

BINARIES="libstdc++-6.dll libwinpthread-1.dll libgcc_s_sjlj-1.dll libcrypto-10.dll libssl-10.dll libpng16-16.dll liblzma-5.dll libharfbuzz-0.dll libpcre-1.dll libintl-8.dll iconv.dll libpcre16-0.dll libfreetype-6.dll libbz2-1.dll libjpeg-62.dll libEGL.dll libglib-2.0-0.dll libGLESv2.dll zlib1.dll Qt5Core.dll Qt5Gui.dll Qt5Network.dll Qt5Qml.dll Qt5Quick.dll Qt5Widgets.dll Qt5WinExtras.dll libiso9660io.dll"
PLUGINS="imageformats/qjpeg.dll platforms/qwindows.dll"
QMLMODULES="Qt QtQml QtQuick/Controls QtQuick/Dialogs QtQuick/Extras QtQuick/Layouts QtQuick/PrivateWidgets QtQuick/Window.2 QtQuick.2"

INSTALL_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PREFIX)
BIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_BINS)
PLUGIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PLUGINS)
QML_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_QML)


mkdir -p "$BUILDPATH"
pushd "$BUILDPATH" >/dev/null

if [ "$1" == "local" ]; then
    if [ "$2" == "debug" ]; then
        INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-$(git describe --tags)-debug.exe"
    else
        INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-$(git describe --tags).exe"
    fi
else
    INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-$(rpm -q mingw32-mediawriter --queryformat '%{VERSION}\n').exe"
fi


if [ "$1" == "local" ]; then
    echo "=== Building"
    if [ "$2" == "debug" ]; then
        mingw32-qmake-qt5 .. CONFIG+=debug
    else
        mingw32-qmake-qt5 ..
    fi

    mingw32-make -j9 >/dev/null
else
    mkdir -p "app/release"
    echo "=== Getting distribution binary"
    cp "$BIN_PREFIX/mediawriter.exe" app/release
    cp "$INSTALL_PREFIX/libexec/mediawriter/helper.exe" app/
fi

pushd "app/release" >/dev/null

if [ ! -f "$MEDIAWRITER" ] || [ ! -f "$HELPER" ]; then
    echo "$MEDIAWRITER or $HELPER doesn't exist."
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

if [ "$2" == "debug" ]; then
    for i in $BINARIES; do
        mkdir -p $(dirname $i)
        cp -r "${BIN_PREFIX}/${i}.debug" "$(dirname $i)"
    done
fi

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

#echo "=== Compressing binaries"
#upx $(find . -name "*.exe")
#upx $(find . -name "*.dll")

# See http://stackoverflow.com/questions/18287960/signing-windows-application-on-linux-based-distros for details
echo "=== Signing binaries"

osslsigncode sign -pkcs12 $CERTPATH/authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.verisign.com/scripts/timstamp.dll -in "$MEDIAWRITER" -out "$MEDIAWRITER.signed" >/dev/null
mv "$MEDIAWRITER.signed" "$MEDIAWRITER"

osslsigncode sign -pkcs12 $CERTPATH/authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.verisign.com/scripts/timstamp.dll -in "$HELPER" -out "$HELPER.signed" >/dev/null
mv "$HELPER.signed" "$HELPER"

cp "$HELPER" .


popd >/dev/null
popd >/dev/null

echo "=== Composing installer"
unix2dos < "$ROOTPATH/LICENSE" > "$BUILDPATH/app/release/LICENSE.txt"
makensis "$SCRIPTDIR/mediawriter.nsi" >/dev/null
mv "$SCRIPTDIR/FMW-setup.exe" "$INSTALLER"

osslsigncode sign -pkcs12 $CERTPATH//authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.verisign.com/scripts/timstamp.dll -in "$INSTALLER" -out "$INSTALLER.signed" >/dev/null
mv "$INSTALLER.signed" "$INSTALLER"

echo "=== Installer is located in $INSTALLER"
