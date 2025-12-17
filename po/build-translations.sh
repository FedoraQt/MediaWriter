#!/bin/bash

####### App translations
rm -f *.qm

echo -e '<RCC>\n\t<qresource prefix="/translations/">' > ../src/translations/translations.qrc
for i in `ls mediawriter_*.po`; do
    echo $i
    LANGCODE=$(sed 's/mediawriter_\([^.]*\).po/\1/' <<< "$i")
    if [[ "$LANGCODE" = "pt-BR" || "$LANGCODE" = "zh-CN" || "$LANGCODE" = "zh-TW" ]]; then
        LANGCODE=${LANGCODE/-/_}
    fi
    lrelease-qt6 $i -qm ../src/translations/$LANGCODE.qm
    echo -e "\t\t<file>${LANGCODE}.qm</file>" >> ../src/translations/translations.qrc
done
echo -e '\t</qresource>\n</RCC>' >> ../src/translations/translations.qrc

####### Appstream metadata
for i in `ls mediawriter_*.po`; do
    echo $i
    LANGCODE=$(sed 's/mediawriter_\([^.]*\).po/\1/' <<< "$i")
    msgfmt $i -o "${LANGCODE}.mo"
done

itstool -i as-metainfo.its -j ../src/app/data/org.bazzite.MediaWriter.metainfo.xml.in -o ../src/app/data/org.bazzite.MediaWriter.metainfo.xml *.mo

rm -f *.mo

####### Desktop file
mkdir -p desktop-file
cp -r *.po desktop-file

pushd desktop-file

for i in `ls mediawriter_*.po`; do
    echo $i
    LANGCODE=$(sed 's/mediawriter_\([^.]*\).po/\1/' <<< "$i")
    mv "$i" "$LANGCODE.po"
done

intltool-merge -d . ../../src/app/data/org.bazzite.MediaWriter.desktop.in ../../src/app/data/org.bazzite.MediaWriter.desktop
popd

rm -rf desktop-file

