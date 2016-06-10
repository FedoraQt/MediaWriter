#!/bin/sh

FILE=$1

APPID=`basename $FILE .json`

echo ========== Building $APPID ================
flatpak-builder --force-clean --ccache --require-changes --repo=repo --subject="Nightly build of ${APPID}, `date`" ${EXPORT_ARGS-} app $FILE

