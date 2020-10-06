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
import MediaWriter 1.0

FocusScope {
    id: root
    property bool open: false
    visible: opacity > 0.0
    enabled: open
    opacity: open ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 120 } }

    onOpenChanged: {
        if (open)
            hideTimer.start()
    }

    height: contents.height + (2 * units.largeSpacing)
    width: contents.width + (2 * units.largeSpacing)

    default property alias children: contents.data

    Timer {
        id: hideTimer
        repeat: false
        interval: 10000
        onTriggered: {
            open = false
        }
    }

    MouseArea {
        x: -mainWindow.width
        y: -mainWindow.height
        width: 2 * mainWindow.width
        height: 2 * mainWindow.height
        onClicked: {
            root.open = false
        }
    }

    Rectangle {
        z: -2
        anchors {
            fill: frame
            topMargin: -1
            bottomMargin: -2
            leftMargin: -2
            rightMargin: -2
        }

        radius: frame.radius + 2
        color: "#10000000"
    }

    Rectangle {
        id: frame
        anchors.fill: contents
        anchors.margins: -(2 * units.largeSpacing)
        color: palette.highlight
        antialiasing: true
        border {
            width: 1
            color: Qt.darker(palette.highlight, 1.0)
        }
        radius: units.smallSpacing
        Rectangle {
            z: -1
            y: -units.largeSpacing - 1
            antialiasing: true
            border.color:  Qt.darker(palette.highlight, 1.0)
            border.width: 1
            color: palette.highlight
            anchors.left: parent.left
            anchors.leftMargin: parent.radius + Math.round(units.gridUnit / 2)
            width: units.gridUnit
            height: units.gridUnit
            rotation: 45
        }
        Rectangle {
            color: palette.highlight
            y: -units.largeSpacing + 1
            anchors.left: parent.left
            anchors.leftMargin: parent.radius + Math.round(units.gridUnit / 2)
            width: units.gridUnit
            height: units.gridUnit
            rotation: 45
        }
    }

    Item {
        id: contents
        width: childrenRect.width
        height: childrenRect.height
    }
}
