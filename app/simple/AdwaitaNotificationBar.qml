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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

Rectangle {
    id: root
    clip: true
    enabled: open
    focus: open

    color: "#729FCF"
    border {
        color: "#4C7BB2"
        width: 1
    }

    height: $(70)
    property real margin: open ? 0 : $(-70)
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
            margins: $(18)
        }
        spacing: $(12)
        Text {
            id: label

            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pointSize: $$(9)
            color: "white"
            textFormat: Text.RichText
        }

        AdwaitaButton {
            id: buttonAccept
            visible: text.length

            onClicked: {
                root.open = false
                root.accepted()
            }

            Layout.alignment: Qt.AlignVCenter
        }

        AdwaitaButton {
            id: buttonCancel
            flat: true
            color: "transparent"
            implicitWidth: $(20)
            implicitHeight: $(20)
            Cross {
                color: palette.windowText
                anchors.centerIn: parent
                width: $(8)
                height: $(8)
            }

            onClicked: {
                root.open = false
                root.cancelled()
            }

            Layout.alignment: Qt.AlignVCenter
        }
    }
}

