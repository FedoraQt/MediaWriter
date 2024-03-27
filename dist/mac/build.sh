#!/bin/bash

set -x
set -e

# TODO: add your Qt location into LD_LIBRARY_PATH
PATH="/usr/local/opt/qt/6.5.2/macos/bin:$PATH"
LD_LIBRARY_PATH="/usr/local/opt/qt/6.5.2/macos/lib:$LD_LIBRARY_PATH"
Qt6_DIR="/usr/local/opt/qt/6.5.2/macos"

# === SIGNING ===
# TODO: set in order to sign binaries
DEVELOPER_ID=""
NOTARIZATION_KEYCHAIN_ITEM="XCODE_NOTARY"
NOTARIZATION_ITUNES_ORGID=""
NOTARIZATION_EMAIL=""

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

cd "${SCRIPTDIR}/../.."

if [[ "$TAG_NAME" == "" ]]; then
    VERSION=$(git rev-parse HEAD | cut -c 1-8)
else
    VERSION="$TAG_NAME"
fi

INSTALLER="$SCRIPTDIR/AOSCMediaWriter-osx-$VERSION.dmg"

function install_deps() {
    brew install cmake
    brew install xz
    mkdir -p /usr/local/opt/qt
    pushd /usr/local/opt/qt >/dev/null
    pip3 install aqtinstall
    aqt install-qt mac desktop 6.5.2 -m qtimageformats
    popd >/dev/null
}

function build() {
    rm -fr "build"
    mkdir -p "build"

    echo "=== Building ==="
    pushd build >/dev/null
    cmake .. -DCREATE_STANDALONE_MAC_BUNDLE=true
    make -j9
    popd >/dev/null
}

function deps() {
    pushd build >/dev/null
    echo "=== Checking unresolved library deps ==="
    # Look at the binaries and search for dynamic library dependencies that are not included on every system
    # So far, this finds only liblzma but in the future it may be necessary for more libs
    for binary in "helper" "AOSC Media Writer"; do
        otool -L "src/app/AOSC Media Writer.app/Contents/MacOS/$binary" |\
            grep -E "^\s" | grep -Ev "AppKit|Metal|Foundation|OpenGL|AGL|DiskArbitration|IOKit|ImageIO|libc\+\+|libobjc|libSystem|@rpath|$(basename $binary)" |\
            sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
            while read library; do
            if [[ ! $library == @loader_path/* ]]; then
                echo "Copying $(basename $library)"
                # fix for newer version of Mac
                if [[ "$library" == "/usr/lib/liblzma.5.dylib" && ! -e "$library" ]]; then
                    cp "/usr/local/lib/liblzma.5.dylib" "src/app/AOSC Media Writer.app/Contents/Frameworks"
                else
                    cp $library "src/app/AOSC Media Writer.app/Contents/Frameworks"
                fi
                install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "src/app/AOSC Media Writer.app/Contents/MacOS/$binary"
            fi
        done
    done
    popd >/dev/null
}

function sign() {
    pushd build >/dev/null
    echo "=== Signing the package ==="
    # sign all frameworks and then the package
    find src/app/AOSC\ Media\ Writer.app -name "*framework" | while read framework; do
        codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/" -o runtime
    done

    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/AOSC\ Media\ Writer.app/Contents/MacOS/AOSC\ Media\ Writer -o runtime
    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/AOSC\ Media\ Writer.app/Contents/MacOS/helper -o runtime
    codesign -s "$DEVELOPER_ID" --deep -v -f src/app/AOSC\ Media\ Writer.app/ -o runtime
    popd >/dev/null
}

function dmg() {
    pushd build >/dev/null
    echo "=== Creating a disk image ==="
    # create the .dmg package - beware, it won't work while FMW is running (blocks partition mounting)
    rm -f "../*.dmg"
    STAGING_DIR="$SCRIPTDIR/staging"
    mkdir -p $STAGING_DIR
    cp -R src/app/AOSC\ Media\ Writer.app $STAGING_DIR
    ln -s /Applications $STAGING_DIR/Applications
    hdiutil create -srcfolder $STAGING_DIR -format UDCO -imagekey zlib-level=9 -scrub -volname AOSCMediaWriter-osx ../AOSCMediaWriter-osx-$VERSION.unnotarized.dmg
    rm -rf $STAGING_DIR
    popd >/dev/null
}

function notarize() {
    echo "=== Submitting for notarization ==="
    xcrun altool --notarize-app --primary-bundle-id "io.aosc.mediawriter" --username "${NOTARIZATION_EMAIL}" --password "@keychain:${NOTARIZATION_KEYCHAIN_ITEM}" --asc-provider "${NOTARIZATION_ITUNES_ORGID}" --file "../AOSCMediaWriter-osx-$VERSION.unnotarized.dmg"

    echo "DONE. After notarization finished (you'll get an email), run:"
    echo "$ xcrun stabler stable app/AOSC\ Media\ Writer.app"
    echo "$ hdiutil create -srcfolder app/AOSC\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname AOSCMediaWriter-osx ../AOSCMediaWriter-osx-$VERSION.dmg"
}

if [[ $# -eq 0 ]]; then
    install_deps
    build
    deps
    sign
    dmg
    notarize
else
    $1
fi
