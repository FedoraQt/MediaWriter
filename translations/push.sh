#!/bin/bash

rm -f mediawriter.pot mediawriter.ts
lupdate-qt5 ../mediawriter.pro -ts mediawriter.ts
lconvert-qt5 -of po -o mediawriter.pot mediawriter.ts
xgettext ../dist/linux/mediawriter.desktop -o desktop.pot
itstool -i as-metainfo.its -o appstream.pot ../dist/linux/mediawriter.appdata.xml.in
zanata-cli push -B
