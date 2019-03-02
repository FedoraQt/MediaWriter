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

import QtQuick 2.4
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

import "../simple"
import "../complex"

FocusScope {
    id: imageList

    property alias currentIndex: osListView.currentIndex
    property real fadeDuration: 200
    property int lastIndex: -1

    property bool focused: contentList.currentIndex === 0
    signal stepForward(int index)
    onStepForward: lastIndex = index
    enabled: focused

    anchors.fill: parent
    clip: true

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.ForwardButton
        onClicked: {
            if (lastIndex >= 0 && mouse.button == Qt.ForwardButton)
                stepForward(lastIndex)
        }
    }

    // this has to be here for softwarecontext (clipping is a bit broken)
    Rectangle {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: searchBox.bottom
            rightMargin: fullList.viewport ? fullList.width - fullList.viewport.width : 0
        }
        z: 1
        color: palette.window
    }

    Rectangle {
        enabled: !releases.frontPage
        opacity: !releases.frontPage ? 1.0 : 0.0
        visible: opacity > 0.0
        id: searchBox
        border {
            color: searchInput.activeFocus ? "#4a90d9" : Qt.darker(palette.button, 1.3)
            width: 1
        }
        radius: $(5)
        color: palette.background
        anchors {
            top: parent.top
            left: parent.left
            right: archSelect.left
            topMargin: $(12)
            leftMargin: mainWindow.margin
            rightMargin: $(4)
        }
        height: $(36)
        z: 2

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
                color: palette.text
                Rectangle {
                    height: $(7)
                    antialiasing: true
                    width: height
                    radius: height / 2
                    color: palette.background
                    anchors.centerIn: parent
                }
                Rectangle {
                    height: $(2)
                    width: $(6)
                    radius: $(2)
                    x: $(8)
                    y: $(11)
                    rotation: 45
                    color: palette.text
                }
            }
        }
        TextInput {
            id: searchInput
            activeFocusOnTab: searchBox.visible
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
                font.pointSize: $$(9)
                text: qsTr("Find an operating system image")
                visible: !parent.activeFocus && parent.text.length == 0
                verticalAlignment: Text.AlignVCenter
            }
            verticalAlignment: TextInput.AlignVCenter
            text: releases.filterText
            onTextChanged: releases.filterText = text
            clip: true
            color: palette.text
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
        activeFocusOnTab: visible
        visible: opacity > 0.0
        anchors {
            right: parent.right
            top: parent.top
            rightMargin: mainWindow.margin + $(1)
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

    Row {
        anchors.top: whiteBackground.bottom
        anchors.right: whiteBackground.right
        anchors.topMargin: $(3)
        anchors.rightMargin: $(5)
        opacity: releases.beingUpdated ? 0.8 : 0.0
        visible: opacity > 0.01
        spacing: $(3)
        Behavior on opacity { NumberAnimation { } }

        BusyIndicator {
            anchors.verticalCenter: parent.verticalCenter
            height: checkingForUpdatesText.height * 0.8
            width: height
        }
        Text {
            id: checkingForUpdatesText
            text: qsTr("Checking for new releases")
            font.pointSize: $$(9)
            color: "#7a7a7a"
        }
    }

    ScrollView {
        id: fullList
        anchors.fill: parent
        ListView {
            id: osListView
            clip: true
            focus: true
            anchors {
                fill: parent
                leftMargin: mainWindow.margin
                rightMargin: anchors.leftMargin - (fullList.width - fullList.viewport.width)
                topMargin: whiteBackground.y
            }

            model: releases

            delegate: DelegateImage {
                width: parent.width
                focus: true
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
                        font.pointSize: $$(9)
                        color: palette.windowText
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
                id: footerRoot
                height: !releases.frontPage ? aboutColumn.height + $(72) : $(36)
                width: osListView.width
                z: 0
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
                            font.pointSize: $$(9)
                            color: palette.windowText
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
                        color: palette.background
                        border {
                            color: Qt.darker(palette.background, 1.3)
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
                                font.pointSize: $$(9)
                                color: palette.text
                            }
                            Text {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                visible: releases.beingUpdated
                                text: qsTr("Fedora Media Writer is now checking for new releases")
                                font.pointSize: $$(9)
                                BusyIndicator {
                                    anchors.right: parent.left
                                    anchors.rightMargin: $(3)
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: parent.height - $(3)
                                    width: height
                                }
                                color: palette.text
                            }
                            Text {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/FedoraQt/MediaWriter/issues\">https://github.com/FedoraQt/MediaWriter/</a>")
                                textFormat: Text.RichText
                                font.pointSize: $$(9)
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
            }

            Rectangle {
                id: threeDotWrapper
                clip: true
                visible: releases.frontPage
                enabled: visible
                activeFocusOnTab: true
                radius: $(3)
                color: palette.window
                width: osListView.width - $(2)
                height: $(32)
                anchors.horizontalCenter: parent.horizontalCenter
                y: $(84)*3 + 1
                z: -1
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: $(-10)
                    color: threeDotMouse.containsPress ? Qt.darker(palette.window, 1.2) : threeDotMouse.containsMouse ? palette.window : palette.background
                    Behavior on color { ColorAnimation { duration: 120 } }
                    radius: $(5)
                    border {
                        color: Qt.darker(palette.background, 1.3)
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
                    Repeater {
                        model: 3
                        Rectangle { height: $(4); width: $(4); radius: $(1); color: mixColors(palette.windowText, palette.window, 0.75); antialiasing: true }
                    }
                }

                Text {
                    id: threeDotText
                    y: threeDotDots.hidden ? parent.height / 2 - height / 2 : -height
                    font.pointSize: $$(9)
                    anchors.horizontalCenter: threeDotDots.horizontalCenter
                    Behavior on y { NumberAnimation { duration: 60 } }
                    clip: true
                    text: qsTr("Display additional Fedora flavors")
                    color: "gray"
                }

                FocusRectangle {
                    visible: threeDotWrapper.activeFocus
                    anchors.fill: parent
                    anchors.margins: $(2)
                }

                Timer {
                    id: threeDotTimer
                    interval: 200
                    onTriggered: {
                        threeDotDots.hidden = true
                    }
                }

                Keys.onSpacePressed: threeDotMouse.action()
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
                    function action() {
                        moveUp.enabled = true
                        releases.frontPage = false
                    }
                    onClicked: {
                        action()
                    }
                }
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
                color: Qt.darker(palette.window, 1.2)
                implicitWidth: $(11)
                implicitHeight: $(11)
            }
            handle: Rectangle {
                color: mixColors(palette.window, palette.windowText, 0.5)
                x: $(3)
                y: $(3)
                implicitWidth: $(6)
                implicitHeight: $(7)
                radius: $(4)
            }
            transientScrollBars: false
            handleOverlap: $(-2)
            minimumHandleLength: $(10)
        }
    }
}
