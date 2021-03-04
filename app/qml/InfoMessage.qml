/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 * Copyright (C) 2020 Jan Grulich <jgrulich@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import MediaWriter 1.0

RowLayout {
    id: root
    property alias text: infoMessageText.text
    property bool error: false
    spacing: units.largeSpacing

    Icon {
        source: "qrc:/icons/dialog-information"
        visible: !root.error
        Layout.fillWidth: false
        Layout.alignment: Qt.AlignVCenter
        width: units.gridUnit
        height: units.gridUnit
    }

    Icon {
        source: "qrc:/icons/dialog-error"
        visible: root.error
        Layout.fillWidth: false
        Layout.alignment: Qt.AlignVCenter
        width: units.gridUnit
        height: units.gridUnit
    }

    QQC2.Label {
        id: infoMessageText
        Layout.fillHeight: true
        Layout.fillWidth: true
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        textFormat: Text.RichText
        onLinkActivated: Qt.openUrlExternally(link)
        MouseArea {
            acceptedButtons: Qt.NoButton
            anchors.fill: parent
            cursorShape: infoMessageText.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }
}
