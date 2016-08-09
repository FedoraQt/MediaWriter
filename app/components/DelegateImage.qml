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
import MediaWriter 1.0

Item {
    id: root
    width: parent.width
    height: $(84)

    readonly property bool isTop: !releases.get(index-1) || (release.category !== releases.get(index-1).category)
    readonly property bool isBottom:
        typeof release !== 'undefined' &&
        !releases.frontPage &&
        (typeof releases.get(index+1) == 'undefined' ||
         (release && release.category !== releases.get(index+1).category)
        )

    readonly property color color: delegateMouse.containsPress ? "#ededed" : delegateMouse.containsMouse ? "#f8f8f8" : "white"

    Rectangle {
        width: parent.width - 2
        //height: index == 0 ? parent.height - $(1) : parent.height
        height: parent.height + 1
        x: 1
        //y: index == 0 ? $(1) : 0
        //radius: $(4)
        color: root.color
        border {
            color: "#c3c3c3"
            width: 1
        }
        Item {
            id: iconRect
            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                leftMargin: $(32)
                topMargin: $(16)
                bottomMargin: anchors.topMargin
            }
            width: height
            IndicatedImage {
                fillMode: Image.PreserveAspectFit
                source: release.icon
                sourceSize.height: parent.height
                sourceSize.width: parent.width
            }
        }
        Item {
            id: textRect
            anchors {
                verticalCenter: parent.verticalCenter
                left: iconRect.right
                right: arrow.left
                bottom: parent.bottom
                leftMargin: $(28)
                rightMargin: $(14)
            }
            Text {
                font.pixelSize: $(12)
                text: release.name
                anchors {
                    bottom: parent.verticalCenter
                    left: parent.left
                    bottomMargin: $(2)
                }
                // font.weight: Font.Bold
            }
            Text {
                font.pixelSize: $(12)
                text: release.summary
                anchors {
                    top: parent.verticalCenter
                    left: parent.left
                    right: parent.right
                    topMargin: $(2)
                }
                wrapMode: Text.Wrap
                color: "#a1a1a1"
                // font.weight: Font.Bold
            }
        }
        Arrow {
            id: arrow
            visible: !release.isLocal
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: $(20)
            }
        }
        Rectangle {
            id: topRounding
            visible: root.isTop
            height: $(5)
            color: palette.window
            clip: true
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            Rectangle {
                height: $(10)
                radius: $(5)
                color: root.color
                border {
                    color: "#c3c3c3"
                    width: 1
                }
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
            }
        }
        Rectangle {
            id: bottomRounding
            visible: root.isBottom
            height: $(5)
            color: palette.window
            clip: true
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            Rectangle {
                height: $(10)
                radius: $(5)
                color: root.color
                border {
                    color: "#c3c3c3"
                    width: 1
                }
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }
        }
    }

    MouseArea {
        id: delegateMouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            imageList.currentIndex = index
            imageList.stepForward(release.index)
        }
    }
}
