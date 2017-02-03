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

BusyIndicator {
    id: root
    width: $(148)
    height: $(6)
    property color progressColor: "#54aada"
    property color backgroundColor: Qt.darker(palette.button, 1.2)

    onRunningChanged: {
        if (running) {
            flyingAnimation.stop()
            flyingBar.x = 0
        }
        else {
            flyingAnimation.start()
        }
    }

    Rectangle {
        implicitWidth: root.width
        width: root.width
        height: root.height
        border {
            color: Qt.darker(palette.button, 1.7)
            width: 1
        }
        radius: $(3)
        clip: false
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.backgroundColor, 1.05) }
            GradientStop { position: 1.0; color: Qt.darker(root.backgroundColor, 1.05) }
        }
        Rectangle {
            id: flyingBar
            width: $(32)
            height: root.height - 2
            y: 1
            radius: $(3)
            border {
                color: Qt.darker(palette.button, 1.7)
                width: 1
            }
            opacity: root.running ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { } }
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter(root.progressColor, 1.05) }
                GradientStop { position: 0.9; color: root.progressColor }
                GradientStop { position: 1.0; color: Qt.darker(root.progressColor) }
            }
            SequentialAnimation {
                id: flyingAnimation
                running: root.running
                loops: Animation.Infinite
                NumberAnimation {
                    duration: 1000
                    target: flyingBar
                    easing.type: Easing.InOutCubic
                    property: "x"
                    from: 1
                    to: root.width - flyingBar.width
                }
                NumberAnimation {
                    duration: 1000
                    target: flyingBar
                    easing.type: Easing.InOutCubic
                    property: "x"
                    from: root.width - flyingBar.width
                    to: 1
                }
            }
        }
    }

    // this style is completely useless, let's just implement it outside
    style: BusyIndicatorStyle {
        indicator: Item {
        }
    }
}
