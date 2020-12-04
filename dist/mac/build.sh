#!/bin/bash

set -x
set -e

PATH="/usr/local/opt/qt/bin:/usr/local/opt/git/bin:/usr/local/bin:$PATH"

DEVELOPER_ID="Developer ID Application: Martin Briza (Z52EFCPL6D)"
QT_ROOT="/usr/local/opt/qt"
QMAKE="${QT_ROOT}/bin/qmake"
MACDEPLOYQT="${QT_ROOT}/bin/macdeployqt"
NOTARIZATION_EMAIL=""
NOTARIZATION_KEYCHAIN_ITEM="XCODE_NOTARY"
NOTARIZATION_ITUNES_ORGID=""

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

cd "${SCRIPTDIR}/../.."

if [[ "$TAG_NAME" == "" ]]; then
    VERSION=$(git describe --tags --always)
else
    VERSION="$TAG_NAME"
fi

INSTALLER="$SCRIPTDIR/FedoraMediaWriter-osx-$VERSION.dmg"

function configure() {
    rm -fr "build"
    mkdir -p "build"
    pushd build >/dev/null

    echo "=== Building Adwaita-qt ==="
    git clone https://github.com/FedoraQt/adwaita-qt.git
    pushd adwaita-qt > /dev/null
    mkdir -p "build"
    pushd build > /dev/null
    cmake ..
    make -j9
    make install
    popd >/dev/null
    popd >/dev/null
    
    echo "=== Building ==="
    ${QMAKE} .. "PKG_CONFIG=PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:/usr/local/opt/qt/lib/pkgconfig/ /usr/local/bin/pkg-config"
    popd >/dev/null
}

function build() {
    pushd build >/dev/null
    make -j9
    popd >/dev/null
}

function deps() {
    pushd build >/dev/null
    echo "=== Inserting Qt deps ==="
    cp "helper/mac/helper.app/Contents/MacOS/helper" "app/Fedora Media Writer.app/Contents/MacOS"
    ${MACDEPLOYQT} "app/Fedora Media Writer.app" -qmldir=".." -executable="app/Fedora Media Writer.app/Contents/MacOS/helper"

    echo "=== Inserting Adwaita-qt library ==="
    cp "/usr/local/lib/libadwaitaqt.1.dylib" "app/Fedora Media Writer.app/Contents/Frameworks"
    cp "/usr/local/lib/libadwaitaqtpriv.1.dylib" "app/Fedora Media Writer.app/Contents/Frameworks"

    echo "=== Checking unresolved library deps ==="
    # Look at the binaries and search for dynamic library dependencies that are not included on every system
    # So far, this finds only liblzma but in the future it may be necessary for more libs
    for binary in "app/Fedora Media Writer.app/Contents/MacOS/helper" "app/Fedora Media Writer.app/Contents/MacOS/Fedora Media Writer" \
                  "app/Fedora Media Writer.app/Contents/Frameworks/libadwaitaqt.1.dylib" \
                  "app/Fedora Media Writer.app/Contents/Frameworks/libadwaitaqtpriv.1.dylib"; do
        otool -L "$binary" |\
            grep -E "^\s" | grep -Ev "Foundation|OpenGL|AGL|DiskArbitration|IOKit|libc\+\+|libobjc|libSystem|@rpath|$(basename $binary)" |\
            sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
            while read library; do
            if [[ ! $library == @loader_path/* ]]; then
                if [[ ! $library == *Qt* ]]; then
                    echo "Copying $(basename $library)"
                    cp -f $library "app/Fedora Media Writer.app/Contents/Frameworks"
                    install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "$binary"
                else
                    install_name_tool -change "$library" "@loader_path/../Frameworks/$(basename ${library}).framework/$(basename ${library})" "$binary"
                fi
            fi
        done
    done
    
    popd >/dev/null
}

function sign() {
    pushd build >/dev/null
    echo "=== Signing the package ==="
    # sign all frameworks and then the package
    find app/Fedora\ Media\ Writer.app -name "*framework" | while read framework; do
        codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/" -o runtime
    done
    codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/Contents/MacOS/Fedora\ Media\ Writer -o runtime --entitlements ../app/Entitlements.plist
    codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/Contents/MacOS/helper -o runtime --entitlements ../app/Entitlements.plist
    codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/ -o runtime --entitlements ../app/Entitlements.plist
    popd >/dev/null
}

function dmg() {
    pushd build >/dev/null
    echo "=== Creating a disk image ==="
    # create the .dmg package - beware, it won't work while FMW is running (blocks partition mounting)
    rm -f "../FedoraMediaWriter-osx-$VERSION.dmg"
    hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$VERSION.unnotarized.dmg
    popd >/dev/null
}

function notarize() {
    echo "=== Submitting for notarization ==="
    xcrun altool --notarize-app --primary-bundle-id "org.fedoraproject.mediawriter" --username "${NOTARIZATION_EMAIL}" --password "@keychain:${NOTARIZATION_KEYCHAIN_ITEM}" --asc-provider "${NOTARIZATION_ITUNES_ORGID}" --file "../FedoraMediaWriter-osx-$VERSION.unnotarized.dmg"

    echo "DONE. After notarization finished (you'll get an email), run:"
    echo "$ xcrun stabler stable app/Fedora\ Media\ Writer.app"
    echo "$ hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$VERSION.dmg"
}

if [[ $# -eq 0 ]]; then
    configure
    build
    deps
    sign
    dmg
    notarize
else
    $1
fi
