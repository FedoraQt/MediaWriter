#!/bin/bash

DEVELOPER_ID="Mac Developer: Martin Briza (N952V7G2F5)"
QT_ROOT="${HOME}/Qt/5.8/clang_64"
QMAKE="${QT_ROOT}/bin/qmake"
MACDEPLOYQT="${QT_ROOT}/bin/macdeployqt"

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

cd "${SCRIPTDIR}/../.."

INSTALLER="$SCRIPTDIR/FedoraMediaWriter-osx-$(git describe --tags).dmg"

rm -fr "build"
mkdir -p "build"
pushd build >/dev/null

echo "=== Building dependency [iso9660io] ==="
# TODO(squimrel): Use latest release once available.
git clone --depth 1 https://github.com/squimrel/iso9660io
pushd iso9660io >/dev/null
cmake "-DCMAKE_INSTALL_PREFIX:PATH=${PWD}" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 .
make install
export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${PWD}"
mkdir -p "../app/Fedora Media Writer.app/Contents/Frameworks/"
mv "$(ls libiso9660io.*.dylib | tail -n 1)" "../app/Fedora Media Writer.app/Contents/Frameworks/"
popd >/dev/null

echo "=== Building dependency [isomd5sum] ==="
# TODO(squimrel): Use latest release once PRs are merged and released.
git clone --depth 1 -b cooking https://github.com/squimrel/isomd5sum
pushd isomd5sum >/dev/null
cmake "-DCMAKE_INSTALL_PREFIX:PATH=${PWD}" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 .
make install
export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${PWD}"
popd >/dev/null


echo "=== Building ==="
${QMAKE} .. >/dev/null
make -j9 >/dev/null

echo "=== Inserting Qt deps ==="
mv "app/helper.app/Contents/MacOS/helper" "app/Fedora Media Writer.app/Contents/MacOS/helper"
${MACDEPLOYQT} "app/Fedora Media Writer.app" -qmldir="../app" -executable="app/Fedora Media Writer.app/Contents/MacOS/helper"

echo "=== Checking unresolved library deps ==="
# Look at the binaries and search for dynamic library dependencies that are not included on every system
# So far, this finds only liblzma but in the future it may be necessary for more libs
for binary in "helper" "Fedora Media Writer"; do 
    otool -L "app/Fedora Media Writer.app/Contents/MacOS/$binary" |\
        grep -E "^\s" | grep -Ev "Foundation|OpenGL|AGL|DiskArbitration|IOKit|libc\+\+|libobjc|libSystem|@rpath" |\
        sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
        while read library; do
        echo "Copying $(basename $library)"
        cp $library "app/Fedora Media Writer.app/Contents/Frameworks"
        install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "app/Fedora Media Writer.app/Contents/MacOS/$binary"    
    done
done

echo "=== Signing the package ==="
# sign all frameworks and then the package
find app/Fedora\ Media\ Writer.app -name "*framework" | while read framework; do
    codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/"
done
codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/

echo "=== Creating a disk image ==="
# create the .dmg package - beware, it won't work while FMW is running (blocks partition mounting)
rm -f ../FedoraMediaWriter-osx-$(git describe --tags).dmg
hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$(git describe --tags).dmg

popd >/dev/null
