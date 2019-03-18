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
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0

import MediaWriter 1.0

import "../simple"
import "../complex"

Item {
    id: root
    anchors.fill: parent

    property bool focused: contentList.currentIndex === 1
    enabled: focused

    onFocusedChanged: {
        if (focused && !prereleaseNotification.wasOpen && releases.selected.prerelease.length > 0)
            prereleaseTimer.start()
    }

    Connections {
        target: focused && releases.selected ? releases.selected : null
        onPrereleaseChanged: {
            if (releases.selected.prerelease.length > 0)
                prereleaseTimer.start()
        }
    }

    function toMainScreen() {
        archPopover.open = false
        versionPopover.open = false
        canGoBack = false
        contentList.currentIndex--
    }

    signal stepForward

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.BackButton
        onClicked:
            if (mouse.button == Qt.BackButton)
                toMainScreen()
    }

    ScrollView {
        activeFocusOnTab: false
        focus: true
        anchors {
            fill: parent
            leftMargin: anchors.rightMargin
        }
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        flickableItem.flickableDirection: Flickable.VerticalFlick
        contentItem: Item {
            x: mainWindow.margin
            width: root.width - 2 * mainWindow.margin
            height: childrenRect.height + $(64) + $(32)

            ColumnLayout {
                y: $(18)
                width: parent.width
                spacing: $(24)

                RowLayout {
                    id: tools
                    Layout.fillWidth: true
                    BackButton {
                        id: backButton
                        onClicked: toMainScreen()
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    AdwaitaButton {
                        text: qsTr("Create Live USB…")
                        color: "#628fcf"
                        textColor: "white"
                        onClicked: {
                            if (dlDialog.visible)
                                return
                            deviceNotification.open = false
                            archPopover.open = false
                            versionPopover.open = false
                            dlDialog.visible = true
                            releases.variant.download()
                        }
                        enabled: !releases.selected.isLocal || releases.variant.iso
                    }
                }

                RowLayout {
                    z: 1 // so the popover stays over the text below
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: $(24)
                    Item {
                        Layout.preferredWidth: $(64) + $(16)
                        Layout.preferredHeight: $(64)
                        IndicatedImage {
                            x: $(12)
                            source: releases.selected.icon ? releases.selected.icon: ""
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                        }
                    }
                    ColumnLayout {
                        Layout.fillHeight: true
                        spacing: $(6)
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                Layout.fillWidth: true
                                anchors.left: parent.left
                                font.pointSize: $$(14)
                                text: releases.selected.name
                                color: palette.windowText
                            }
                            Text {
                                font.pointSize: $$(12)
                                property double size: releases.variant.size
                                text: size <= 0 ? "" :
                                      (size < 1024) ? (size + " B") :
                                      (size < (1024 * 1024)) ? ((size / 1024).toFixed(1) + " KB") :
                                      (size < (1024 * 1024 * 1024)) ? ((size / 1024 / 1024).toFixed(1) + " MB") :
                                      ((size / 1024 / 1024 / 1024).toFixed(1) + " GB")

                                color: mixColors(palette.window, palette.windowText, 0.3)
                            }
                            Text {
                                font.pointSize: $$(12)
                                visible: releases.variant.realSize != releases.variant.size && releases.variant.realSize > 0.1
                                property double size: releases.variant.realSize
                                property string sizeString: size <= 0 ? "" :
                                                           (size < 1024) ? (size + " B") :
                                                           (size < (1024 * 1024)) ? ((size / 1024).toFixed(1) + " KB") :
                                                           (size < (1024 * 1024 * 1024)) ? ((size / 1024 / 1024).toFixed(1) + " MB") :
                                                           ((size / 1024 / 1024 / 1024).toFixed(1) + " GB")
                                //: The downloaded image is compressed, this refers to the size which it will take after being decompressed and written to the drive
                                text: qsTr("(%1 after writing)").arg(sizeString)
                                color: mixColors(palette.window, palette.windowText, 0.3)
                            }
                        }
                        ColumnLayout {
                            width: parent.width
                            spacing: $(6)
                            opacity: releases.selected.isLocal ? 0.0 : 1.0
                            Text {
                                font.pointSize: $$(10)
                                color: mixColors(palette.window, palette.windowText, 0.3)
                                visible: typeof releases.selected.version !== 'undefined'
                                text: releases.variant.name
                            }
                            Text {
                                font.pointSize: $$(8)
                                color: mixColors(palette.window, palette.windowText, 0.3)
                                visible: releases.selected.version && releases.variant
                                text: releases.variant.arch.details
                            }
                            RowLayout {
                                spacing: 0
                                width: parent.width
                                Text {
                                    text: qsTr("Version %1").arg(releases.selected.version.name)
                                    font.pointSize: $$(8)

                                    color: versionRepeater.count <= 1 ? mixColors(palette.window, palette.windowText, 0.3) : versionMouse.containsPress ? Qt.lighter("#1d61bf", 1.7) : versionMouse.containsMouse ? Qt.darker("#1d61bf", 1.5) : "#1d61bf"
                                    Behavior on color { ColorAnimation { duration: 100 } }
                                    MouseArea {
                                        id: versionMouse
                                        activeFocusOnTab: true
                                        enabled: versionRepeater.count > 1
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        function action() {
                                            versionPopover.open = !versionPopover.open
                                        }
                                        onClicked: {
                                            action()
                                        }
                                        Keys.onSpacePressed: action()
                                        FocusRectangle {
                                            anchors.fill: parent
                                            anchors.margins: $(-2)
                                            visible: parent.activeFocus
                                        }
                                    }

                                    BusyIndicator {
                                        anchors.right: parent.left
                                        anchors.rightMargin: $(4)
                                        anchors.verticalCenter: parent.verticalCenter
                                        height: parent.height * 0.8
                                        width: height
                                        opacity: releases.beingUpdated ? 0.6 : 0.0
                                        visible: opacity > 0.01
                                        Behavior on opacity { NumberAnimation { } }
                                    }

                                    Rectangle {
                                        visible: versionRepeater.count > 1
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                            top: parent.bottom
                                        }
                                        radius: height / 2
                                        color: parent.color
                                        antialiasing: true
                                        height: $(1)
                                    }


                                    AdwaitaPopOver {
                                        id: versionPopover
                                        z: 2
                                        anchors {
                                            horizontalCenter: parent.horizontalCenter
                                            top: parent.bottom
                                            topMargin: $(8) + opacity * $(24)
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                prereleaseNotification.open = false
                                                archPopover.open = false
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: $(9)
                                            ExclusiveGroup {
                                                id: versionEG
                                            }
                                            Repeater {
                                                id: versionRepeater
                                                model: releases.selected.versions
                                                AdwaitaRadioButton {
                                                    text: name
                                                    Layout.alignment: Qt.AlignVCenter
                                                    exclusiveGroup: versionEG
                                                    checked: index == releases.selected.versionIndex
                                                    onCheckedChanged: {
                                                        if (checked)
                                                            releases.selected.versionIndex = index
                                                        versionPopover.open = false
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    AdwaitaPopNotification {
                                        id: prereleaseNotification
                                        z: 2
                                        property bool wasOpen: false
                                        anchors {
                                            left: parent.left
                                            top: parent.bottom
                                            topMargin: $(8) + opacity * $(24)
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                versionPopover.open = false
                                                archPopover.open = false
                                            }
                                        }

                                        Text {
                                            text: qsTr("Fedora %1 was released! Check it out!<br>If you want a stable, finished system, it's better to stay at version %2.").arg(releases.selected.prerelease).arg(releases.selected.version.name)
                                            font.pointSize: $$(8)
                                            color: "white"
                                        }

                                        Timer {
                                            id: prereleaseTimer
                                            interval: 300
                                            repeat: false
                                            onTriggered: {
                                                prereleaseNotification.open = true
                                                prereleaseNotification.wasOpen = true
                                            }
                                        }
                                    }
                                }
                                Text {
                                    // I'm sorry, everyone, I can't find a better way to determine if the date is valid
                                    visible: releases.selected.version.releaseDate.toLocaleDateString().length > 0
                                    text: qsTr(", released on %1").arg(releases.selected.version.releaseDate.toLocaleDateString())
                                    font.pointSize: $$(8)
                                    color: mixColors(palette.window, palette.windowText, 0.3)
                                }
                                Item {
                                    Layout.fillWidth: true
                                }
                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    visible: releases.selected.version.variants.length > 1
                                    text: qsTr("Other variants...")
                                    font.pointSize: $$(8)
                                    color: archMouse.containsPress ? Qt.lighter("#1d61bf", 1.7) : archMouse.containsMouse ? Qt.darker("#1d61bf", 1.5) : "#1d61bf"
                                    Behavior on color { ColorAnimation { duration: 100 } }
                                    MouseArea {
                                        id: archMouse
                                        activeFocusOnTab: parent.visible
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        function action() {
                                            if (versionPopover.open) {
                                                versionPopover.open = false
                                            }
                                            else {
                                                archPopover.open = !archPopover.open
                                            }
                                        }
                                        Keys.onSpacePressed: action()
                                        onClicked: {
                                            action()
                                        }
                                        FocusRectangle {
                                            anchors.fill: parent
                                            anchors.margins: $(-2)
                                            visible: parent.activeFocus
                                        }
                                    }

                                    Rectangle {
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                            top: parent.bottom
                                        }
                                        radius: height / 2
                                        color: parent.color
                                        height: $(1)
                                    }

                                    AdwaitaPopOver {
                                        id: archPopover
                                        z: 2
                                        anchors {
                                            horizontalCenter: parent.horizontalCenter
                                            top: parent.bottom
                                            topMargin: $(8) + opacity * $(24)
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                versionPopover.open = false
                                                prereleaseNotification.open = false
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: $(9)
                                            ExclusiveGroup {
                                                id: archEG
                                            }
                                            Repeater {
                                                model: releases.selected.version.variants
                                                AdwaitaRadioButton {
                                                    text: name
                                                    Layout.alignment: Qt.AlignVCenter
                                                    exclusiveGroup: archEG
                                                    checked: index == releases.selected.version.variantIndex
                                                    onCheckedChanged: {
                                                        if (checked)
                                                            releases.selected.version.variantIndex = index
                                                        archPopover.open = false
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    width: Layout.width
                    wrapMode: Text.WordWrap
                    text: releases.selected.description
                    textFormat: Text.RichText
                    font.pointSize: $$(9)
                    color: palette.windowText
                }
                Repeater {
                    id: screenshotRepeater
                    model: releases.selected.screenshots
                    ZoomableImage {
                        z: 0
                        smooth: true
                        cache: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: width / sourceSize.width * sourceSize.height
                        fillMode: Image.PreserveAspectFit
                        source: modelData
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
            handleOverlap: $(1)
            minimumHandleLength: $(10)
        }
    }
}
