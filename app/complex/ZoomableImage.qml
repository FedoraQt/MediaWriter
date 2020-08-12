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
import QtQuick.Window 2.12
import "../simple"

IndicatedImage {
    id: root
    activeFocusOnTab: true

    Keys.onSpacePressed: mouse.action()

    MouseArea {
        id: mouse
        anchors.fill: parent
        function action() {
            fullscreenViewer.show(root.source)
        }
        onClicked: action()
        cursorShape: Qt.PointingHandCursor
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: root.zoomed ? 0.8 : 0.0
        Behavior on opacity { NumberAnimation { duration: 160 } }
    }

    FocusRectangle {
        anchors.fill: parent
        anchors.margins: units.smallSpacing
        visible: parent.activeFocus
    }

    Behavior on scale {
        NumberAnimation {
            easing.type: Easing.InOutElastic
        }
    }
}
