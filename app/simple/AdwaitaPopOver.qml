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
import QtQuick.Window 2.0

FocusScope {
    id: popover
    property bool open: false
    visible: opacity > 0.0
    opacity: open ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 120 } }

    height: contents.height + $(12)
    width: contents.width + $(12)

    default property alias children: contents.data

    MouseArea {
        x: -mainWindow.width
        y: -mainWindow.height
        width: 2 * mainWindow.width
        height: 8 * mainWindow.height
        onClicked: {
            popover.open = false
        }
    }

    Rectangle {
        z: -2
        anchors {
            fill: frame
            topMargin: -$(1)
            bottomMargin: -$(2)
            leftMargin: -$(2)
            rightMargin: -$(2)
        }

        radius: frame.radius + $(2)
        color: "#10000000"
    }

    Rectangle {
        id: frame
        anchors.fill: contents
        anchors.margins: - $(12)
        color: palette.window
        antialiasing: true
        border {
            width: 1
            color: Qt.darker(palette.button, 1.5)
        }
        radius: $(6)
        Rectangle {
            z: -1
            y: -$(6.5) - 1
            antialiasing: true
            border {
                width: 1
                color: Qt.darker(palette.button, 1.5)
            }
            color: palette.window
            anchors.horizontalCenter: parent.horizontalCenter
            width: $(14)
            height: $(14)
            rotation: 45
        }
        Rectangle {
            color: palette.window
            y: -$(6.5) + 1
            anchors.horizontalCenter: parent.horizontalCenter
            width: $(14)
            height: $(14)
            rotation: 45
        }
        MouseArea { // to stay open when user clicks inside the bubble
            anchors.fill: parent
        }
    }

    Item {
        id: contents
        width: childrenRect.width
        height: childrenRect.height
    }
}
