#!/bin/bash

cd $(dirname $0)/..
rm -f po/mediawriter.pot mediawriter.ts
lupdate-qt6 src/app/qml.qrc src/app/*.cpp src/app/*.h src/helper/linux/*.cpp src/helper/mac/*.cpp src/helper/win/*.cpp -ts mediawriter.ts
lconvert-qt6 -of po -o po/app.pot mediawriter.ts
xgettext src/app/data/org.fedoraproject.MediaWriter.desktop -o po/desktop.pot
itstool -i po/as-metainfo.its -o po/appstream.pot src/app/data/org.fedoraproject.MediaWriter.appdata.xml.in
msgcat po/*.pot > po/mediawriter.pot
rm po/app.pot po/appstream.pot po/desktop.pot
