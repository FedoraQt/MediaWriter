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

${QMAKE} .. >/dev/null
make -j9 >/dev/null

cp "helper/mac/helper.app/Contents/MacOS/helper" "app/Fedora Media Writer.app/Contents/MacOS"
${MACDEPLOYQT} "app/Fedora Media Writer.app" -qmldir="../app" -executable="app/Fedora Media Writer.app/Contents/MacOS/helper"

for binary in "helper" "Fedora Media Writer"; do 
    otool -L "app/Fedora Media Writer.app/Contents/MacOS/$binary" |\
        grep -E "^\s" | grep -Ev "Foundation|OpenGL|AGL|DiskArbitration|IOKit|libc\+\+|libobjc|libSystem|@rpath" |\
        sed -e 's/[[:space:]]\([^[:space:]]*\).*/\1/' |\
        while read library; do
        
        cp $library "app/Fedora Media Writer.app/Contents/Frameworks"
        install_name_tool -change "$library" "@executable_path/../Frameworks/$(basename ${library})" "app/Fedora Media Writer.app/Contents/MacOS/$binary"    
    done
done
 
find app/Fedora\ Media\ Writer.app -name "*framework" | while read framework;
 do
   codesign -s "$DEVELOPER_ID" --deep -v -f "$framework/Versions/Current/"
 done
 
codesign -s "$DEVELOPER_ID" --deep -v -f app/Fedora\ Media\ Writer.app/


rm -f ../FedoraMediaWriter-osx-$(git describe --tags).dmg
hdiutil create -srcfolder app/Fedora\ Media\ Writer.app  -format UDCO -imagekey zlib-level=9 -scrub -volname FedoraMediaWriter-osx ../FedoraMediaWriter-osx-$(git describe --tags).dmg

# mv "$PWD/app/Fedora Media Writer.dmg" "$INSTALLER"
# echo "$INSTALLER"

popd >/dev/null
