#!/bin/bash

rm -f *.qm *.po

zanata-cli --batch-mode pull

echo -e '<RCC>' > translations.qrc
echo -e '\t<qresource prefix="/translations/">' >> translations.qrc
for i in `ls mediawriter-*.po`; do
	sed -i '/"Language:.*/a "X-Qt-Contexts: true\\n"' $i
	LANGCODE=$(sed 's/mediawriter-\([^.]*\).po/\1/' <<< "$i")
	lrelease-qt5 $i -qm $LANGCODE.qm
	echo -e "\t\t<file>${LANGCODE}.qm</file>" >> translations.qrc
done
echo -e '\t</qresource>' >> translations.qrc
echo -e '</RCC>' >> translations.qrc

rm mediawriter-*.po

for i in `ls appstream-*.po`; do
    echo $i
    LANGCODE=$(sed 's/appstream-\([^.]*\).po/\1/' <<< "$i")
    msgfmt $i -o "${LANGCODE}.mo"
done

itstool -i as-metainfo.its -j ../dist/linux/mediawriter.appdata.xml.in -o ../dist/linux/mediawriter.appdata.xml *.mo

rm appstream-*.po *.mo

for i in `ls desktop-*.po`; do
    echo $i
    LANGCODE=$(sed 's/desktop-\([^.]*\).po/\1/' <<< "$i")
    mv $i "$LANGCODE.po"
done

intltool-merge -d . ../dist/linux/mediawriter.desktop.in ../dist/linux/mediawriter.desktop

rm *.po
