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
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import MediaWriter 1.0

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
            if (lastIndex >= 0 && mouse.button === Qt.ForwardButton)
                stepForward(lastIndex)
        }
    }

    RowLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: 12
            leftMargin: mainWindow.margin
            rightMargin: mainWindow.margin + 1
        }
        spacing: 4

        QQC2.TextField {
            id: searchBox
            Layout.fillWidth: true
            z: 2
            enabled: !releases.frontPage
            opacity: !releases.frontPage ? 1.0 : 0.0
            visible: opacity > 0.0
            activeFocusOnTab: visible
            placeholderText: qsTr("Find an operating system image")
            text: releases.filterText
            onTextChanged: releases.filterText = text
            clip: true

            Behavior on opacity {
                NumberAnimation {
                    duration: imageList.fadeDuration
                }
            }
        }

        QQC2.ComboBox {
            id: archSelect
            enabled: !releases.frontPage
            opacity: !releases.frontPage ? 1.0 : 0.0
            activeFocusOnTab: visible
            visible: opacity > 0.0
            model: releases.architectures

            onCurrentIndexChanged:  {
                releases.filterArchitecture = currentIndex
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: imageList.fadeDuration
                }
            }
        }
    }

    Rectangle {
        id: whiteBackground

        z: -1
        clip: true
        radius: 6
        color: "transparent"
        y: releases.frontPage || moveUp.running ? parent.height / 2 - height / 2 : 54
        Behavior on y {
            id: moveUp
            enabled: false

            NumberAnimation {
                onStopped: moveUp.enabled = false
            }
        }
        height: releases.frontPage ? adjustedHeight() : parent.height
        anchors {
            left: parent.left
            right: parent.right
            rightMargin: mainWindow.margin
            leftMargin: anchors.rightMargin
        }

        function adjustedHeight() {
            var height = Math.round(units.gridUnit * 4.5) * 3 + (units.gridUnit * 2)
            if (height % 2) {
                return height + 1
            } else {
                return height
            }
        }
    }

    Row {
        anchors.top: whiteBackground.bottom
        anchors.right: whiteBackground.right
        anchors.topMargin: units.smallSpacing
        anchors.rightMargin: units.largeSpacing
        opacity: releases.beingUpdated ? 0.8 : 0.0
        visible: opacity > 0.01
        spacing: units.smallSpacing
        Behavior on opacity { NumberAnimation { } }

        // TODO: Adwaita themed component
        QQC2.BusyIndicator {
            anchors.verticalCenter: checkingForUpdatesText.verticalCenter
            height: 24
            width: 24
        }
        QQC2.Label {
            id: checkingForUpdatesText
            text: qsTr("Checking for new releases")
            opacity: 0.6
        }
    }

    QQC2.ScrollView {
        id: fullList
        anchors {
            fill: parent
            topMargin: whiteBackground.y
        }

        ListView {
            id: osListView
            anchors {
                fill: parent
                leftMargin: mainWindow.margin
                rightMargin: mainWindow.margin
            }

            clip: true
            focus: true
            model: releases

            delegate: DelegateImage {
                width: osListView.width
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
                    height: section == "main" ? 0 : Math.round(units.gridUnit * 3.5)
                    width: parent.width
                    QQC2.Label {
                        text: section
                        textFormat: Text.RichText
                        anchors {
                            left: parent.left
                            bottom: parent.bottom
                            leftMargin: units.gridUnit
                            bottomMargin: units.largeSpacing + units.smallSpacing
                        }
                    }
                }
            }

            footer: Item {
                id: footerRoot
                height: !releases.frontPage ? aboutColumn.height + (units.gridUnit * 4) : units.gridUnit * 2
                width: osListView.width
                z: 0
                Column {
                    id: aboutColumn
                    width: parent.width
                    visible: !releases.frontPage
                    spacing: 0
                    Item {
                        width: parent.width
                        height: Math.round(units.gridUnit * 3.5)

                        QQC2.Label {
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                leftMargin: units.gridUnit
                                bottomMargin: units.largeSpacing + units.smallSpacing
                            }
                            text: qsTr("About Fedora Media Writer")
                        }
                    }
                    Rectangle {
                        width: parent.width
                        radius: 5
                        color: palette.background
                        border {
                            color: Qt.darker(palette.background, 1.3)
                            width: 1
                        }
                        height: childrenRect.height + Math.round(units.gridUnit * 1.3)
                        Behavior on height { NumberAnimation {} }
                        Column {
                            id: aboutLayout
                            spacing: units.smallSpacing
                            y: units.largeSpacing + units.smallSpacing
                            x: units.gridUnit * 2
                            width: parent.width
                            move: Transition { NumberAnimation { properties: "y" } }

                            QQC2.Label {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: qsTr("Version %1").arg(mediawriterVersion)
                                textFormat: Text.RichText
                            }
                            QQC2.Label {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                visible: releases.beingUpdated
                                text: qsTr("Fedora Media Writer is now checking for new releases")

                                // TODO: Adwaita themed component
                                QQC2.BusyIndicator {
                                    anchors.right: parent.left
                                    anchors.rightMargin: units.smallSpacing
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: 24
                                    width: 24
                                }
                            }
                            QQC2.Label {
                                width: parent.width
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/FedoraQt/MediaWriter/issues\">https://github.com/FedoraQt/MediaWriter/</a>")
                                textFormat: Text.RichText
                                onLinkActivated: Qt.openUrlExternally(link)
                                opacity: 0.6

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
                radius: 3
                color: palette.window
                width: osListView.width - 2
                height: units.gridUnit * 2
                anchors.horizontalCenter: parent.horizontalCenter
                y: Math.round(units.gridUnit * 4.5) * 3 + 1
                z: -1
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: -units.largeSpacing
                    color: threeDotMouse.containsPress ? Qt.darker(palette.window, 1.2) : threeDotMouse.containsMouse ? palette.window : palette.background
                    Behavior on color { ColorAnimation { duration: 120 } }
                    radius: 5
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
                    spacing: units.smallSpacing
                    Repeater {
                        model: 3
                        Rectangle { height: 4; width: 4; radius: 1; color: mixColors(palette.windowText, palette.window, 0.75); antialiasing: true }
                    }
                }

                QQC2.Label {
                    id: threeDotText
                    y: threeDotDots.hidden ? parent.height / 2 - height / 2 : -height
                    anchors.horizontalCenter: threeDotDots.horizontalCenter
                    Behavior on y { NumberAnimation { duration: 60 } }
                    clip: true
                    text: qsTr("Display additional Fedora flavors")
                    opacity: 0.6
                }

                FocusRectangle {
                    visible: threeDotWrapper.activeFocus
                    anchors.fill: parent
                    anchors.margins: 2
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
    }
}
