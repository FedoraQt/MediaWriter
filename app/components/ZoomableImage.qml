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

import QtQuick 2.0
import QtQuick.Window 2.0

IndicatedImage {
    id: root
    property bool zoomed: false
    onZoomedChanged: {
        if (zoomed) {
            mainWindow.visibility = Window.FullScreen
            z++
            showAnimation.start()
        }
        else {
            mainWindow.visibility = Window.Windowed
            z--
            hideAnimation.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.zoomed = !root.zoomed
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: root.zoomed ? 0.8 : 0.0
        Behavior on opacity { NumberAnimation { duration: 160 } }
    }

    Image {
        id: zoomedImage
        source: root.source
        visible: false
        SequentialAnimation {
            id: showAnimation
            onStarted: {
                zoomedImage.visible = true
            }

            ParallelAnimation {
                NumberAnimation {
                    target: zoomedImage; property: "width"
                    from: root.width
                    to: Screen.width
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "height"
                    from: root.height
                    to: Screen.height
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "x"
                    from: 0
                    to: (root.width - Screen.width) / 2 + mainWindow.potentialMargin - mainWindow.margin
                    easing.type: Easing.OutCubic
                }
            }

            onStopped: {
                zoomedImage.x = mapFromItem(null, 0, 0).x - (zoomedImage.width - Screen.width) / 2
                zoomedImage.y = mapFromItem(null, 0, 0).y - (zoomedImage.height - Screen.height) / 2
            }
        }

        SequentialAnimation {
            id: hideAnimation
            onStopped: {
                zoomedImage.visible = false
            }

            ParallelAnimation {
                NumberAnimation {
                    target: zoomedImage; property: "width"
                    to: root.width
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "height"
                    to: root.height
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "x"
                    to: 0
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "y"
                    to: 0
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on height { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on width { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on x { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on y { NumberAnimation { easing.type: Easing.OutCubic } }

        MouseArea {
            id: mouse
            enabled: root.zoomed
            anchors.fill: parent
            onClicked: root.zoomed = false
            onWheel: {
                if(wheel.angleDelta.y > 0 && zoomedImage.width < 10 * zoomedImage.sourceSize.width) {
                    zoomedImage.x -= zoomedImage.width * 0.15
                    zoomedImage.y -= zoomedImage.height * 0.15
                    zoomedImage.width *= 1.3
                    zoomedImage.height *= 1.3
                }
                else if (wheel.angleDelta.y < 0) {
                    if (zoomedImage.width * 0.7 < root.width) {
                        root.zoomed = false
                    }
                    else {
                        zoomedImage.x += zoomedImage.width * 0.15
                        zoomedImage.y += zoomedImage.height * 0.15
                        zoomedImage.width *= 0.7
                        zoomedImage.height *= 0.7
                    }
                }
            }
            drag {
                target: zoomedImage
                axis: Drag.XAndYAxis
                minimumX: - width + 32
                maximumX: (root.width - 32)
                minimumY: - height + 32
                maximumY: (root.height - 32)
            }
        }
    }

    Behavior on scale {
        NumberAnimation {
            easing.type: Easing.InOutElastic
        }
    }
}
