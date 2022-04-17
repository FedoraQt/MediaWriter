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
    # Bug in macdeployqt
    # HACK https://github.com/Homebrew/discussions/discussions/2823
    pushd "/usr/local/lib/QtGui.framework/Versions/A"
    install_name_tool -id '@rpath/QtGui.framework/Versions/A/QtGui' 'QtGui'
    popd >/dev/null

    rm -fr "build"
    mkdir -p "build"
    pushd build >/dev/null
    
    echo "=== Building ==="
    cmake .. -DCREATE_STANDALONE_MAC_BUNDLE=true 
    popd >/dev/null
}

function build() {
    pushd build >/dev/null
    make -j9
    popd >/dev/null
}

function deps() {
    pushd build >/dev/null
    echo "=== Checking unresolved library deps ==="
    # Look at the binaries and search for dynamic library dependencies that are not included on every system
    # So far, this finds only liblzma but in the future it may be necessary for more libs
    for binary in "helper" "Fedora Media Writer"; do
        otool -L "src/app/Fedora Media Writer.app/Contents/MacOS/$binary" |\
            grep -E "^\s" | grep -Ev "AppKit|Metal|Foundation|OpenGL|AGL|DiskArbitration|IOKit|libc\+\+|libobjc|libSystem|@rpath|$(basename $binary)" |\
            sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
            while read library; do
            if [[ ! $library == @loader_path/* ]]; then
                echo "Copying $(basename $library)"
                if [[ "$library" == "/System/Library/Frameworks/ImageIO.framework/Versions/A/ImageIO" && ! -e "$library" ]]; then
                    continue
                fi
                # fix for newer version of Mac
                if [[ "$library" == "/usr/lib/liblzma.5.dylib" && ! -e "$library" ]]; then
                    library="/usr/local/lib/liblzma.5.dylib"
                fi
                cp $library "src/app/Fedora Media Writer.app/Contents/Frameworks"
                install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "src/app/Fedora Media Writer.app/Contents/MacOS/$binary"
            fi
        done
    done
    echo "Copy libbrotlicommon.1.dylib"
    cp "/usr/local/opt/brotli/lib/libbrotlicommon.1.dylib" "src/app/Fedora Media Writer.app/Contents/Frameworks"        
    popd >/dev/null
}

function sign() {
    pushd build >/dev/null
    echo "=== Signing the package ==="
    # sign all frameworks and then the package
    find src/app/Fedora\ Media\ Writer.app -name "*framework" | while read framework; do
        codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/" -o runtime
    done

    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/Fedora\ Media\ Writer.app/Contents/MacOS/Fedora\ Media\ Writer -o runtime
    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/Fedora\ Media\ Writer.app/Contents/MacOS/helper -o runtime
    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/Fedora\ Media\ Writer.app/ -o runtime
    popd >/dev/null
}

function dmg() {
    pushd build >/dev/null
    echo "=== Creating a disk image ==="
    # create the .dmg package - beware, it won't work while FMW is running (blocks partition mounting)
    rm -f "../FedoraMediaWriter-osx-$VERSION.unnotarized.dmg"
    hdiutil create -srcfolder src/app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$VERSION.unnotarized.dmg
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
