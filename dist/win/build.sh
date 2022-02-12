#!/bin/bash -xe


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

opt_local=false
opt_install=false
opt_debug=false
opt_nosign=false

while test $# -gt 0
do
    case "$1" in
        local) opt_local=true
            ;;
        install) opt_install=true
            ;;
        debug) opt_debug=true
            ;;
        nosign) opt_nosign=true
            ;;
    esac
    shift
done

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

ROOTPATH=$(realpath "$SCRIPTDIR/../../")
BUILDPATH="$ROOTPATH/build"

MEDIAWRITER="$BUILDPATH/app/release/mediawriter.exe"
HELPER="$BUILDPATH/app/release/helper.exe"

if ! $opt_nosign; then
    if [ -z "$CERTPATH" ]; then
        CERTPATH=~
    fi
    if [ -z "$CERTPASS" ]; then
        CERTPASS="$CERTPATH/authenticode.pass"
    fi
fi

#mingw32-qt6-qtwinextras
PACKAGES="cmake mingw32-filesystem mingw32-qt6-qtbase mingw32-qt6-qtdeclarative mingw32-xz-libs mingw32-libadwaita-qt6 mingw32-qt6-qtsvg mingw32-nsis osslsigncode wine-core.i686 mingw32-angleproject wine-systemd"

if ! $opt_local; then
    PACKAGES="$PACKAGES mingw32-mediawriter"
fi

if $opt_install; then
    echo "=== Installing dependencies"
    dnf install $PACKAGES
    exit $?
fi

echo "=== Checking dependencies"
DEPENDENCIES=0
for i in $PACKAGES; do
    rpm -V $i
    if [ $? -ne 0 ]; then
        if [ "$i" == "osslsigncode" ]; then
            opt_nosign=true
        else
            DEPENDENCIES=1
        fi
    fi
done
if [ $DEPENDENCIES -ne 0 ]; then exit 1; fi

BINARIES="libstdc++-6.dll libgcc_s_dw2-1.dll libssp-0.dll iconv.dll libwinpthread-1.dll libcrypto-1_1.dll libssl-1_1.dll libpng16-16.dll liblzma-5.dll libharfbuzz-0.dll libpcre-1.dll libintl-8.dll iconv.dll libpcre2-16-0.dll libfreetype-6.dll libbz2-1.dll libjpeg-62.dll libEGL.dll libglib-2.0-0.dll libGLESv2.dll zlib1.dll libadwaitaqt6priv-1.dll libadwaitaqt6-1.dll Qt6Core.dll Qt6Gui.dll Qt6Network.dll Qt6QmlWorkerScript.dll Qt6Svg.dll Qt6WinExtras.dll"

# BINARIES="libstdc++-6.dll libgcc_s_dw2-1.dll libssp-0.dll iconv.dll libwinpthread-1.dll libcrypto-1_1.dll libssl-1_1.dll libpng16-16.dll liblzma-5.dll libharfbuzz-0.dll libpcre-1.dll libintl-8.dll iconv.dll libpcre2-16-0.dll libfreetype-6.dll libbz2-1.dll libjpeg-62.dll libEGL.dll libglib-2.0-0.dll libGLESv2.dll zlib1.dll libadwaitaqt6priv-1.dll libadwaitaqt6-1.dll Qt6Core.dll Qt6Gui.dll Qt6Network.dll Qt6Widgets.dll Qt6Declarative.dll"

PLUGINS="imageformats/qjpeg.dll imageformats/qsvg.dll platforms/qwindows.dll"
#PLUGINS="imageformats/qjpeg.dll"

QMLMODULES="Qt QtQml QtQuick/Controls QtQuick/Controls.2 QtQuick/Dialogs QtQuick/Extras QtQuick/Layouts QtQuick/PrivateWidgets QtQuick/Shapes QtQuick/Templates.2 QtQuick/Window.2 QtQuick.2"
#QMLMODULES="Qt QtQml QtQuick/Controls QtQuick/Controls.2 QtQuick/Dialogs QtQuick/Layouts QtQuick/Shapes"

# INSTALL_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PREFIX)
# BIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_BINS)
# PLUGIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PLUGINS)
# QML_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_QML)

INSTALL_PREFIX=$(mingw32-cmake -L | grep CMAKE_INSTALL_PREFIX | cut -d "=" -f2)
BIN_PREFIX=$(mingw32-cmake -L | grep CMAKE_INSTALL_PREFIX | cut -d "=" -f2)
PLUGIN_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_PLUGINS | tr 5 6)
QML_PREFIX=$(mingw32-qmake-qt5 -query QT_INSTALL_QML | tr 5 6)

export WINEPREFIX="$BUILDPATH/wineprefix"
export WINEDEBUG="-all"

mkdir -p "$BUILDPATH"
mkdir -p "$WINEPREFIX"
pushd "$BUILDPATH" >/dev/null

VERSION_FULL=''

if $opt_local; then
    VERSION_FULL=$(git describe --tags)
    if $opt_debug; then
        INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-${VERSION_FULL}-debug.exe"
    else
        INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-${VERSION_FULL}.exe"
    fi
else
    VERSION_FULL=$(rpm -q mingw32-mediawriter --queryformat '%{VERSION}\n')
    INSTALLER="$SCRIPTDIR/FedoraMediaWriter-win32-${VERSION_FULL}.exe"
fi

VERSION_STRIPPED=$(sed "s/-.*//" <<< "${VERSION_FULL}")
VERSION_MAJOR=$(cut -d. -f1 <<< "${VERSION_STRIPPED}")
VERSION_MINOR=$(cut -d. -f2 <<< "${VERSION_STRIPPED}")
VERSION_BUILD=$(cut -d. -f3 <<< "${VERSION_STRIPPED}")

if $opt_local; then
    echo "=== Building"
    if [ "2" == "debug" ]; then
        mingw32-cmake ..
    else
        mingw32-cmake ..
    fi

    mingw32-make -j9 > /dev/null

    # FIXME just a workaround for Adwaita theme not being build and placed to correct location
    # without installation
#     mkdir -p $BUILDPATH/app/release/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme
#     mkdir -p $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme
#     cp -r ../src/theme/qml/* $BUILDPATH/app/release/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme
#     cp -r ../src/theme/qmldir $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme
#     cp -r $BUILDPATH/src/theme/adwaitathemeplugin.dll $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme
    cp -r $BUILDPATH/src/app/helper.exe $BUILDPATH/app/release/
    cp -r $BUILDPATH/src/app/mediawriter.exe $BUILDPATH/app/release/
else
    mkdir -p "app/release"
    echo "=== Getting distribution binary"
    cp "$BIN_PREFIX/bin/mediawriter.exe" app/release
    cp "$INSTALL_PREFIX/libexec/mediawriter/helper.exe" app/release
#     mkdir -p $BUILDPATH/app/release/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme
#     mkdir -p $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme
#     cp -r $QML_PREFIX/org/fedoraproject/AdwaitaTheme/* $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme/
#     cp -r $QML_PREFIX/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/* $BUILDPATH/app/release/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/
#     cp -r /usr/lib64/qt6/qml/org/fedoraproject/AdwaitaTheme/* $BUILDPATH/app/release/org/fedoraproject/AdwaitaTheme/
#     cp -r /usr/lib64/qt6/qml/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/* $BUILDPATH/app/release/QtQuick/Controls.2/org.fedoraproject.AdwaitaTheme/
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
    cp -r "${BIN_PREFIX}/bin/${i}" "$(dirname $i)"
done

if $opt_debug; then
    for i in $BINARIES; do
        mkdir -p $(dirname $i)
        cp -r "${BIN_PREFIX}/bin/${i}.debug" "$(dirname $i)"
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
#     cp -r "${QML_PREFIX}/${i}" "$(dirname $i)"
    cp -r "/usr/lib64/qt6/qml/${i}" "$(dirname $i)"
    
done

#echo "=== Compressing binaries"
#upx $(find . -name "*.exe")
#upx $(find . -name "*.dll")

# See http://stackoverflow.com/questions/18287960/signing-windows-application-on-linux-based-distros for details
echo "=== Signing binaries"

if ! $opt_nosign; then
    osslsigncode sign -pkcs12 $CERTPATH/authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "$MEDIAWRITER" -out "$MEDIAWRITER.signed" >/dev/null
    mv "$MEDIAWRITER.signed" "$MEDIAWRITER"

    osslsigncode sign -pkcs12 $CERTPATH/authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "$HELPER" -out "$HELPER.signed" >/dev/null
    mv "$HELPER.signed" "$HELPER"
fi


popd >/dev/null
popd >/dev/null

echo "=== Composing installer"
unix2dos < "$ROOTPATH/LICENSE.GPL-2" > "$BUILDPATH/app/release/LICENSE.GPL-2.txt"
unix2dos < "$ROOTPATH/LICENSE.LGPL-2" > "$BUILDPATH/app/release/LICENSE.LGPL-2.txt"
INSTALLED_SIZE=$(du -k -d0 "$BUILDPATH/app/release" | cut -f1)
cp "$SCRIPTDIR/mediawriter.nsi" "$SCRIPTDIR/mediawriter.tmp.nsi"
sed -i "s/#!define VERSIONMAJOR/!define VERSIONMAJOR ${VERSION_MAJOR}/" "$SCRIPTDIR/mediawriter.tmp.nsi"
sed -i "s/#!define VERSIONMINOR/!define VERSIONMINOR ${VERSION_MINOR}/" "$SCRIPTDIR/mediawriter.tmp.nsi"
sed -i "s/#!define VERSIONBUILD/!define VERSIONBUILD ${VERSION_BUILD}/" "$SCRIPTDIR/mediawriter.tmp.nsi"
sed -i "s/#!define INSTALLSIZE/!define INSTALLSIZE ${INSTALLED_SIZE}/" "$SCRIPTDIR/mediawriter.tmp.nsi"
makensis -DCERTPATH="$CERTPATH" -DCERTPASS="$CERTPASS" "$SCRIPTDIR/mediawriter.tmp.nsi" >/dev/null
rm "$SCRIPTDIR/mediawriter.tmp.nsi"
mv "$SCRIPTDIR/FMW-setup.exe" "$INSTALLER"

if ! $opt_nosign; then
    osslsigncode sign -pkcs12 $CERTPATH//authenticode.pfx -readpass "$CERTPASS" -h sha256 -n "Fedora Media Writer" -i https://getfedora.org -t http://timestamp.comodoca.com/authenticode -in "$INSTALLER" -out "$INSTALLER.signed" >/dev/null
    mv "$INSTALLER.signed" "$INSTALLER"
fi

echo "=== Installer is located in $INSTALLER"
