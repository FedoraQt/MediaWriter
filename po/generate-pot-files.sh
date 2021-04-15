#!/bin/bash

rm -f mediawriter.pot mediawriter.ts
lupdate-qt5 ../mediawriter.pro -ts mediawriter.ts
lconvert-qt5 -of po -o app.pot mediawriter.ts
xgettext ../dist/linux/org.fedoraproject.MediaWriter.desktop -o desktop.pot
itstool -i as-metainfo.its -o appstream.pot ../dist/linux/org.fedoraproject.MediaWriter.appdata.xml.in
json2po -i ../app/assets/metadata.json -o metadata.pot
msgcat *.pot > mediawriter.pot
rm app.pot appstream.pot desktop.pot metadata.pot
