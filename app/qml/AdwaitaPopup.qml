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

Item {
    id: root
    anchors.fill: parent
    property bool open: false
    property alias title: title.text
    property alias text: description.text
    property alias buttonText: button.text
    signal accepted
    onAccepted: root.open = false

    Timer {
        interval: 10000
        onTriggered: root.open = false
        running: !mouse.containsMouse
    }

    Rectangle {
        id: background
        anchors {
            fill: layout
            bottomMargin: -units.gridUnit
            topMargin: anchors.bottomMargin - background.radius
            leftMargin: anchors.bottomMargin
            rightMargin: anchors.bottomMargin + units.smallSpacing
        }
        border {
            color: "#3a3a3a"
            width: 1
        }
        color: "#bb000000"
        radius: units.smallSpacing
        MouseArea {
            id: mouse
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    RowLayout {
        id: layout
        spacing:  units.smallSpacing

        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.666
        y: root.open ? units.gridUnit : -(height + units.gridUnit + units.smallSpacing)
        Behavior on y { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }

        Column {
            Text {
                font.bold: true
                color: "white"
                id: title
            }
            Text {
                color: "white"
                id: description
            }
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.Button {
            id: button
            onClicked: root.accepted()
        }
        // TODO: turn into Toolbutton?
        QQC2.Button {
            id: crossButton
            flat: true
            icon.name: "qrc:/icons/window-close"
            onClicked: root.open = false
        }
    }
}
