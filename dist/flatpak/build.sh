#!/bin/sh

FILE=$1

APPID=`basename $FILE .json`

echo ========== Building $APPID ================
# --require-changes
flatpak-builder --disable-cache --force-clean --ccache --repo=repo --subject="Nightly build of ${APPID}, `date`" ${EXPORT_ARGS-} app $FILE

