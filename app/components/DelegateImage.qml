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
import QtQuick.Layouts 1.0
import MediaWriter 1.0

Item {
    id: root
    width: parent.width
    height: $(84)

    readonly property bool isTop: !releases.get(index-1) || (release.category !== releases.get(index-1).category)
    readonly property bool isBottom:
        typeof release !== 'undefined' &&
        !releases.frontPage &&
        (!releases.get(index+1) ||
         typeof releases.get(index+1) == 'undefined' ||
         (release && release.category !== releases.get(index+1).category)
        )

    property color color: delegateMouse.containsPress ? Qt.darker(palette.window, 1.2) : delegateMouse.containsMouse ? palette.window : palette.base
    Behavior on color { ColorAnimation { duration: 120 } }

    readonly property real animationDuration: 1000

    Rectangle {
        width: parent.width - 2
        height: parent.height + 1
        x: 1
        color: root.color
        border {
            color: Qt.darker(palette.base, 1.3)
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
        ColumnLayout {
            id: textRect
            spacing: $(4)
            anchors {
                verticalCenter: parent.verticalCenter
                left: iconRect.right
                right: arrow.left
                leftMargin: $(28)
                rightMargin: $(14)
            }
            Text {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignBottom
                font.pixelSize: $(12)
                text: release.name
                color: palette.text
            }
            Text {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignTop
                font.pixelSize: $(12)
                text: release.summary
                wrapMode: Text.Wrap
                color: "#a1a1a1"
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
                    color: Qt.darker(palette.base, 1.3)
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
                    color: Qt.darker(palette.base, 1.3)
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
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (release.isLocal) {
                releases.selectedIndex = index
                fileDialog.visible = true
            }
            else {
                imageList.currentIndex = index
                imageList.stepForward(release.index)
            }
        }
    }
}
