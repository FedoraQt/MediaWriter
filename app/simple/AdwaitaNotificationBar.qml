/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
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

Rectangle {
    id: root
    clip: true
    enabled: open
    focus: open

    color: palette.highlight
    border {
        color: Qt.darker(palette.highlight, 1.0)
        width: 1
    }

    height: units.gridUnit * 4
    property real margin: open ? 0 : -(units.gridUnit * 4)
    anchors.topMargin: margin
    Behavior on margin {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutElastic
            easing.amplitude: 0.7
            easing.period: 0.4
        }
    }

    property bool open: false

    property alias text: label.text
    property alias acceptText: buttonAccept.text

    signal accepted
    signal cancelled

    Keys.onEscapePressed: open = false

    RowLayout {
        anchors {
            fill: parent
            margins: units.gridUnit
        }
        spacing: 2 * units.largeSpacing
        QQC2.Label {
            id: label

            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            color: palette.highlightedText
            textFormat: Text.RichText
        }

        QQC2.Button {
            id: buttonAccept
            visible: text.length

            onClicked: {
                root.open = false
                root.accepted()
            }

            Layout.alignment: Qt.AlignVCenter
        }

        // TODO: turn into Toolbutton?
        QQC2.Button {
            id: buttonCancel
            flat: true
            icon.name: "qrc:/icons/window-close"

            onClicked: {
                root.open = false
                root.cancelled()
            }

            Layout.alignment: Qt.AlignVCenter
        }
    }
}

