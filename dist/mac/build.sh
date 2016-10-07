#!/bin/bash

QT_ROOT="${HOME}/Qt/5.7/clang_64"
QMAKE="${QT_ROOT}/bin/qmake"
MACDEPLOYQT="${QT_ROOT}/bin/macdeployqt"

pushd $(dirname $0) >/dev/null
SCRIPTDIR=$(pwd -P)
popd >/dev/null

cd "${SCRIPTDIR}/../.."

INSTALLER="$SCRIPTDIR/FedoraMediaWriter-osx-$(git describe --tags).dmg"

mkdir -p "build"
pushd build >/dev/null

${QMAKE} .. >/dev/null
make -j9 >/dev/null

cp "helper/mac/helper.app/Contents/MacOS/helper" "app/mediawriter.app/Contents/MacOS"
${MACDEPLOYQT} "app/mediawriter.app" -dmg -qmldir="../app" -executable="app/mediawriter.app/Contents/MacOS/helper" -always-overwrite

mv "$PWD/app/mediawriter.dmg" "$INSTALLER"
echo "$INSTALLER"

popd >/dev/null
