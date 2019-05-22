#!/bin/bash

DEVELOPER_ID="Mac Developer: Martin Briza (N952V7G2F5)"
QT_ROOT="${HOME}/Qt/5.12.1/clang_64"
QMAKE="${QT_ROOT}/bin/qmake"
MACDEPLOYQT="${QT_ROOT}/bin/macdeployqt"
NOTARIZATION_EMAIL=""
NOTARIZATION_KEYCHAIN_ITEM="XCODE_NOTARY"
NOTARIZATION_ITUNES_ORGID=""

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

cd "${SCRIPTDIR}/../.."

INSTALLER="$SCRIPTDIR/FedoraMediaWriter-osx-$(git describe --tags).dmg"

rm -fr "build"
mkdir -p "build"
pushd build >/dev/null

echo "=== Building ==="
${QMAKE} .. >/dev/null
make -j9 >/dev/null

echo "=== Inserting Qt deps ==="
cp "helper/mac/helper.app/Contents/MacOS/helper" "app/Fedora Media Writer.app/Contents/MacOS"
${MACDEPLOYQT} "app/Fedora Media Writer.app" -qmldir="../app" -executable="app/Fedora Media Writer.app/Contents/MacOS/helper"

echo "=== Checking unresolved library deps ==="
# Look at the binaries and search for dynamic library dependencies that are not included on every system
# So far, this finds only liblzma but in the future it may be necessary for more libs
for binary in "helper" "Fedora Media Writer"; do 
    otool -L "app/Fedora Media Writer.app/Contents/MacOS/$binary" |\
        grep -E "^\s" | grep -Ev "Foundation|OpenGL|AGL|DiskArbitration|IOKit|libc\+\+|libobjc|libSystem|@rpath" |\
        sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
        while read library; do
        if [[ ! $library == @loader_path/* ]]; then
            echo "Copying $(basename $library)"
            cp $library "app/Fedora Media Writer.app/Contents/Frameworks"
            install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "app/Fedora Media Writer.app/Contents/MacOS/$binary"
        fi
    done
done

echo "=== Signing the package ==="
# sign all frameworks and then the package
find app/Fedora\ Media\ Writer.app -name "*framework" | while read framework; do
    codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/" -o runtime
done
codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/ -o runtime

echo "=== Creating a disk image ==="
# create the .dmg package - beware, it won't work while FMW is running (blocks partition mounting)
rm -f ../FedoraMediaWriter-osx-$(git describe --tags).dmg
hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$(git describe --tags).unnotarized.dmg

echo "=== Submitting for notarization ==="
xcrun altool --notarize-app --primary-bundle-id "org.fedoraproject.mediawriter" --username "${NOTARIZATION_EMAIL}" --password "@keychain:${NOTARIZATION_KEYCHAIN_ITEM}" --asc-provider "${NOTARIZATION_ITUNES_ORGID}" --file "../FedoraMediaWriter-osx-$(git describe --tags).unnotarized.dmg"

echo "DONE. After notarization finished (you'll get an email), run:"
echo "$ xcrun stabler stable app/Fedora\ Media\ Writer.app"
echo "$ hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$(git describe --tags).dmg"

popd >/dev/null
