#!/bin/bash

rm -f *.qm *.po websites/getfedora/*.po websites/spins/*.po websites/labs/*.po

zanata-cli --batch-mode pull &
pushd websites
    pushd getfedora
        zanata-cli --batch-mode pull &
    popd
    pushd spins
        zanata-cli --batch-mode pull &
    popd
    pushd labs
        zanata-cli --batch-mode pull &
    popd
popd

wait

echo -e '<RCC>' > translations.qrc
echo -e '\t<qresource prefix="/translations/">' >> translations.qrc
for i in `ls websites/getfedora/*.po`; do
    LANGCODE=$(basename -s .po $i)
    touch websites/getfedora/$LANGCODE.po websites/labs/$LANGCODE.po websites/spins/$LANGCODE.po mediawriter-$LANGCODE.po
    sed -i '/msgid ["].*/imsgctxt "Release|"' websites/getfedora/$LANGCODE.po
    sed -i '0,/msgctxt .*/{/msgctxt .*/d}' websites/getfedora/$LANGCODE.po
    sed -i '/msgid ["].*/imsgctxt "Release|"' websites/labs/$LANGCODE.po
    sed -i '0,/msgctxt .*/{/msgctxt .*/d}' websites/labs/$LANGCODE.po
    sed -i '/msgid ["].*/imsgctxt "Release|"' websites/spins/$LANGCODE.po
    sed -i '0,/msgctxt .*/{/msgctxt .*/d}' websites/spins/$LANGCODE.po
    msgcat websites/getfedora/$LANGCODE.po websites/labs/$LANGCODE.po websites/spins/$LANGCODE.po mediawriter-$LANGCODE.po -o merged-$LANGCODE.po
    sed -i '/"Language:.*/a "X-Qt-Contexts: true\\n"' merged-$LANGCODE.po
    sed -i '/"#-#-#-#-#.*/d' merged-$LANGCODE.po
    lrelease-qt5 merged-$LANGCODE.po -qm $LANGCODE.qm
    echo -e "\t\t<file>${LANGCODE}.qm</file>" >> translations.qrc
done
echo -e '\t</qresource>' >> translations.qrc
echo -e '</RCC>' >> translations.qrc

rm mediawriter-*.po
rm merged-*.po

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
