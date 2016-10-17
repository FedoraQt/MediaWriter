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

Item {
    id: imageList

    property alias currentIndex: osListView.currentIndex
    property real fadeDuration: 200

    signal stepForward(int index)

    anchors.fill: parent
    clip: true

    Rectangle {
        enabled: !releases.frontPage
        opacity: !releases.frontPage ? 1.0 : 0.0
        id: searchBox
        border {
            color: searchInput.activeFocus ? "#4a90d9" : "#c3c3c3"
            width: 1
        }
        radius: $(5)
        color: "white"
        anchors {
            top: parent.top
            left: parent.left
            right: archSelect.left
            topMargin: $(12)
            leftMargin: mainWindow.margin
            rightMargin: $(4)
        }
        height: $(36)

        Item {
            id: magnifyingGlass
            anchors {
                left: parent.left
                leftMargin: (parent.height - height) / 2
                verticalCenter: parent.verticalCenter
            }
            height: childrenRect.height + $(3)
            width: childrenRect.width + $(2)

            Rectangle {
                height: $(11)
                antialiasing: true
                width: height
                radius: height / 2
                color: "black"
                Rectangle {
                    height: $(7)
                    antialiasing: true
                    width: height
                    radius: height / 2
                    color: "white"
                    anchors.centerIn: parent
                }
                Rectangle {
                    height: $(2)
                    width: $(6)
                    radius: $(2)
                    x: $(8)
                    y: $(11)
                    rotation: 45
                    color: "black"
                }
            }
        }
        TextInput {
            id: searchInput
            anchors {
                left: magnifyingGlass.right
                top: parent.top
                bottom: parent.bottom
                right: parent.right
                margins: $(8)
            }
            Text {
                anchors.fill: parent
                color: "light gray"
                font.pixelSize: $(12)
                text: qsTr("Find an operating system image")
                visible: !parent.activeFocus && parent.text.length == 0
                verticalAlignment: Text.AlignVCenter
            }
            verticalAlignment: TextInput.AlignVCenter
            text: releases.filterText
            onTextChanged: releases.filterText = text
            clip: true
        }
    }

    AdwaitaComboBox {
        enabled: !releases.frontPage
        opacity: !releases.frontPage ? 1.0 : 0.0
        Behavior on opacity {
            NumberAnimation {
                duration: imageList.fadeDuration
            }
        }

        id: archSelect
        anchors {
            right: parent.right
            top: parent.top
            rightMargin: mainWindow.margin
            topMargin: $(12)
        }
        height: $(36)
        width: $(148)
        model: releases.architectures
        onCurrentIndexChanged:  {
            releases.filterArchitecture = currentIndex
        }
    }

    Rectangle {
        id: whiteBackground
        z: -1
        clip: true
        radius: $(6)
        color: "transparent"
        y: releases.frontPage || moveUp.running ? parent.height / 2 - height / 2 : $(54)
        Behavior on y {
            id: moveUp
            enabled: false

            NumberAnimation {
                onStopped: moveUp.enabled = false
            }
        }
        height: releases.frontPage ? $(84) * 3 + $(36) : parent.height
        anchors {
            left: parent.left
            right: parent.right
            rightMargin: mainWindow.margin
            leftMargin: anchors.rightMargin
        }
    }

    ScrollView {
        id: fullList
        anchors.fill: parent
        verticalScrollBarPolicy: releases.frontPage ? Qt.ScrollBarAlwaysOff : Qt.ScrollBarAsNeeded
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        ListView {
            id: osListView
            clip: true
            anchors {
                fill: parent
                leftMargin: mainWindow.margin
                rightMargin: anchors.leftMargin - (fullList.width - fullList.viewport.width)
                topMargin: whiteBackground.y
            }

            section {
                property: "release.category"
                criteria: ViewSection.FullString
                labelPositioning: ViewSection.InlineLabels
                delegate: Item {
                    height: section == "main" ? 0 : $(64)
                    width: parent.width
                    Text {
                        text: section
                        textFormat: Text.RichText
                        font.pixelSize: $(12)
                        anchors {
                            left: parent.left
                            bottom: parent.bottom
                            leftMargin: $(18)
                            bottomMargin: $(12)
                        }
                    }
                }
            }

            footer: Item {
                height: !releases.frontPage ? aboutColumn.height + $(72) : $(36)
                width: osListView.width
                Column {
                    id: aboutColumn
                    width: parent.width
                    visible: !releases.frontPage
                    spacing: 0
                    Item {
                        width: parent.width
                        height: $(64)

                        Text {
                            text: qsTr("About Fedora Media Writer")
                            font.pixelSize: $(12)
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                leftMargin: $(18)
                                bottomMargin: $(12)
                            }
                        }
                    }
                    Rectangle {
                        width: parent.width
                        radius: $(5)
                        color: "#f8f8f8"
                        border {
                            color: "#c3c3c3"
                            width: 1
                        }
                        height: childrenRect.height + $(24)
                        Behavior on height { NumberAnimation {} }
                        Column {
                            id: aboutLayout
                            spacing: $(3)
                            y: $(12)
                            x: $(32)
                            width: parent.width
                            move: Transition { NumberAnimation { properties: "y" } }

                            Text {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: qsTr("Version %1").arg(mediawriterVersion)
                                textFormat: Text.RichText
                                font.pixelSize: $(12)
                            }
                            Text {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                visible: releases.beingUpdated
                                text: qsTr("Fedora Media Writer is now checking for new releases")
                                font.pixelSize: $(12)
                                BusyIndicator {
                                    anchors.right: parent.left
                                    anchors.rightMargin: $(3)
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: parent.height - $(3)
                                    width: height
                                }
                            }
                            Text {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/MartinBriza/MediaWriter\">https://github.com/MartinBriza/MediaWriter</a>")
                                textFormat: Text.RichText
                                font.pixelSize: $(12)
                                onLinkActivated: Qt.openUrlExternally(link)
                                color: Qt.darker("light gray")
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton
                                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    clip: true
                    visible: releases.frontPage
                    anchors.fill: parent
                    anchors.margins: 1
                    radius: $(3)
                    color: palette.window
                    Rectangle {
                        anchors.fill: parent
                        anchors.topMargin: $(-10)
                        color: threeDotMouse.containsPress ? "#e4e4e4" : threeDotMouse.containsMouse ? "#f6f6f6" : "white"
                        Behavior on color { ColorAnimation { duration: 120 } }
                        radius: $(5)
                        border {
                            color: "#c3c3c3"
                            width: 1
                        }
                    }

                    Column {
                        id: threeDotDots
                        property bool hidden: false
                        opacity: hidden ? 0.0 : 1.0
                        Behavior on opacity { NumberAnimation { duration: 60 } }
                        anchors.centerIn: parent
                        spacing: $(3)
                        Rectangle { id: dot1; height: $(4); width: $(4); radius: $(1); color: "#bebebe"; antialiasing: true }
                        Rectangle { id: dot2; height: $(4); width: $(4); radius: $(1); color: "#bebebe"; antialiasing: true }
                        Rectangle { id: dot3; height: $(4); width: $(4); radius: $(1); color: "#bebebe"; antialiasing: true }
                        SequentialAnimation {
                            id: updateAnimation
                            running: releases.beingUpdated
                            loops: -1
                            NumberAnimation { target: dot1; property: "scale"; from: 1.0; to: 1.5; duration: 300 }
                            NumberAnimation { target: dot1; property: "scale"; from: 1.5; to: 1.0; duration: 300 }
                            NumberAnimation { target: dot2; property: "scale"; from: 1.0; to: 1.5; duration: 300 }
                            NumberAnimation { target: dot2; property: "scale"; from: 1.5; to: 1.0; duration: 300 }
                            NumberAnimation { target: dot3; property: "scale"; from: 1.0; to: 1.5; duration: 300 }
                            NumberAnimation { target: dot3; property: "scale"; from: 1.5; to: 1.0; duration: 300 }
                            onStopped: {
                                dot1.scale = 1.0
                                dot2.scale = 1.0
                                dot3.scale = 1.0
                            }
                        }
                    }
                    Text {
                        text: qsTr("Checking for new releases")
                        color: "#adadad"
                        font.pixelSize: $(12)
                        anchors.verticalCenter: threeDotDots.verticalCenter
                        anchors.left: threeDotDots.right
                        anchors.leftMargin: $(6)
                        opacity: !threeDotDots.hidden && releases.beingUpdated ? 1.0 : 0.0
                        Behavior on opacity { NumberAnimation { duration: 60 } }
                    }

                    Text {
                        id: threeDotText
                        y: threeDotDots.hidden ? parent.height / 2 - height / 2 : -height
                        font.pixelSize: $(12)
                        anchors.horizontalCenter: threeDotDots.horizontalCenter
                        Behavior on y { NumberAnimation { duration: 60 } }
                        clip: true
                        text: qsTr("Display additional Fedora flavors")
                        color: "gray"
                    }
                    Timer {
                        id: threeDotTimer
                        interval: 200
                        onTriggered: {
                            threeDotDots.hidden = true
                        }
                    }
                    MouseArea {
                        id: threeDotMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onHoveredChanged: {
                            if (containsMouse && !pressed) {
                                threeDotTimer.start()
                            }
                            if (!containsMouse) {
                                threeDotTimer.stop()
                                threeDotDots.hidden = false
                            }
                        }
                        onClicked: {
                            moveUp.enabled = true
                            releases.frontPage = false
                        }
                    }
                }
            }

            model: releases

            delegate: DelegateImage {
                width: parent.width
            }

            remove: Transition {
                NumberAnimation { properties: "x"; to: width; duration: 300 }
            }
            removeDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 300 }
            }
            add: Transition {
                NumberAnimation { properties: releases.frontPage ? "y" : "x"; from: releases.frontPage ? 0 : -width; duration: 300 }
            }
            addDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 300 }
            }
        }
        style: ScrollViewStyle {
            incrementControl: Item {}
            decrementControl: Item {}
            corner: Item {
                implicitWidth: $(11)
                implicitHeight: $(11)
            }
            scrollBarBackground: Rectangle {
                y: $(-1)
                color: "#dddddd"
                implicitWidth: $(12)
                height: control.height
            }
            handle: Rectangle {
                color: "#b3b5b6"
                x: $(3)
                implicitWidth: $(7)
                implicitHeight: $(7)
                radius: $(4)
            }
            transientScrollBars: false
            handleOverlap: $(-2)
            minimumHandleLength: $(10)
        }
    }
}
