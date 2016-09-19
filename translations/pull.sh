#!/bin/bash

rm -f *.qm *.po

zanata-cli --batch-mode pull

echo -e '<RCC>' > translations.qrc
echo -e '\t<qresource prefix="/translations/">' >> translations.qrc
for i in `ls *.po`; do
	lrelease-qt5 $i
	echo -e "\t\t<file>$i</file>" >> translations.qrc
done
echo -e '\t</qresource>' >> translations.qrc
echo -e '</RCC>' >> translations.qrc
