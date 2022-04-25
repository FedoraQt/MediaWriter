#!/bin/bash

rm -f mediawriter.pot mediawriter.ts
lupdate-qt6 ../src/app/qml.qrc ../src/app/*.cpp -ts mediawriter.ts
lconvert-qt6 -of po -o app.pot mediawriter.ts
xgettext ../src/app/data/org.fedoraproject.MediaWriter.desktop -o desktop.pot
itstool -i as-metainfo.its -o appstream.pot ../src/app/data/org.fedoraproject.MediaWriter.appdata.xml.in
msgcat *.pot > mediawriter.pot
rm app.pot appstream.pot desktop.pot
