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

Item {
    id: root
    implicitHeight: $(36)
    implicitWidth: $(36)

    property alias radius: rect.radius
    property color color: palette.button


    Rectangle {
        id: rect
        anchors.fill: parent
        radius: $(3)

        readonly property int animationDuration: 150
        color: control.enabled ? root.color : "light gray"
        Behavior on color { ColorAnimation { duration: rect.animationDuration } }

        border {
            width: 1
            color: Qt.colorEqual(root.color, "transparent") ? "#212121" : control.enabled ? "#777777" : "#c2c2c2"
        }

        Rectangle {
            id: overlay
            radius: parent.radius - $(1)
            anchors.margins: $(0.5)
            anchors.fill: parent
            //gradient: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ? regularGradient: hoveredGradient : downGradient : disabledGradient
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: overlay.topColor
                    Behavior on color { ColorAnimation { duration: 100 } }
                }
                GradientStop {
                    position: 1.0
                    color: overlay.bottomColor
                    Behavior on color { ColorAnimation { duration: 100 } }
                }
            }

            property color topColor: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ?  "#14ffffff" : "#14ffffff" : "#1e000000" : "transparent"
            property color bottomColor: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ? "#14000000" : "#05ffffff" : "#14000000" : "transparent"
        }
    }

    SystemPalette {
        id: palette
    }
}
