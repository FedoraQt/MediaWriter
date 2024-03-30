#!/bin/bash

####### App translations
rm -f *.qm

echo -e '<RCC>\n\t<qresource prefix="/translations/">' > ../src/translations/translations.qrc
for i in mediawriter-*.po; do
    echo $i
    LANGCODE=$(sed 's/mediawriter-\([^.]*\).po/\1/' <<< "$i")
    lrelease-qt5 $i -qm ../src/translations/$LANGCODE.qm
    echo -e "\t\t<file>${LANGCODE}.qm</file>" >> ../src/translations/translations.qrc
done
echo -e '\t</qresource>\n</RCC>' >> ../src/translations/translations.qrc

####### Appstream metadata
for i in mediawriter-*.po; do
    echo $i
    LANGCODE=$(sed 's/mediawriter-\([^.]*\).po/\1/' <<< "$i")
    msgfmt $i -o "${LANGCODE}.mo"
done

itstool -i as-metainfo.its -j ../src/app/data/io.aosc.MediaWriter.appdata.xml.in -o ../src/app/data/io.aosc.MediaWriter.appdata.xml *.mo

rm -f *.mo

####### Desktop file
mkdir -p desktop-file
cp -r *.po desktop-file

pushd desktop-file

for i in mediawriter-*.po; do
    echo $i
    LANGCODE=$(sed 's/mediawriter-\([^.]*\).po/\1/' <<< "$i")
    mv "$i" "$LANGCODE.po"
done

intltool-merge -d . ../../src/app/data/io.aosc.MediaWriter.desktop.in ../../src/app/data/io.aosc.MediaWriter.desktop
popd

rm -rf desktop-file

