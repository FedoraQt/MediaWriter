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
//import QtQuick.Dialogs 1.3
import QtQuick.Window 2.12

import MediaWriter 1.0

Item {
    id: root
    anchors.fill: parent

    property bool focused: contentList.currentIndex === 1
    enabled: focused

    QQC2.Label {
        id: referenceLabel
        visible: false
        opacity: 0
    }

    onFocusedChanged: {
        if (focused && !prereleaseNotification.wasOpen && releases.selected.prerelease.length > 0)
            prereleaseTimer.start()
    }

    Connections {
        target: focused && releases.selected ? releases.selected : null
        function onPrereleaseChanged() {
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

    QQC2.ScrollView {
        activeFocusOnTab: false
        focus: true
        anchors {
            fill: parent
            leftMargin: anchors.rightMargin
        }

        Item {
            x: mainWindow.margin
            implicitWidth: root.width - 2 * mainWindow.margin
            implicitHeight: childrenRect.height + Math.round(units.gridUnit * 3.5) + units.gridUnit * 2

            ColumnLayout {
                y: units.gridUnit
                width: parent.width
                spacing: units.largeSpacing * 3

                RowLayout {
                    id: tools
                    Layout.fillWidth: true
                    QQC2.Button {
                        id: backButton
                        icon.name: "qrc:/icons/go-previous"
                        text: qsTr("Back")
                        onClicked: toMainScreen()
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    QQC2.Button {
                        text: qsTr("Create Live USB…")
                        highlighted: true

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
                    spacing: units.largeSpacing
                    Item {
                        Layout.preferredWidth: Math.round(units.gridUnit * 3.5) + units.gridUnit
                        Layout.preferredHeight: Math.round(units.gridUnit * 3.5)
                        Layout.alignment: Qt.AlignHCenter
                        IndicatedImage {
                            anchors.fill: parent
                            x: units.gridUnit
                            source: releases.selected.icon ? releases.selected.icon: ""
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: parent.width
                            sourceSize.height: parent.height
                        }
                    }
                    ColumnLayout {
                        Layout.fillHeight: true
                        spacing: units.largeSpacing
                        RowLayout {
                            Layout.fillWidth: true
                            QQC2.Label {
                                Layout.fillWidth: true
                                font.pointSize: referenceLabel.font.pointSize + 4
                                text: releases.selected.name
                            }
                            QQC2.Label {
                                font.pointSize: referenceLabel.font.pointSize + 2
                                property double size: releases.variant.size
                                text: size <= 0 ? "" :
                                                  (size < 1024) ? (size + " B") :
                                                                  (size < (1024 * 1024)) ? ((size / 1024).toFixed(1) + " KB") :
                                                                                           (size < (1024 * 1024 * 1024)) ? ((size / 1024 / 1024).toFixed(1) + " MB") :
                                                                                                                           ((size / 1024 / 1024 / 1024).toFixed(1) + " GB")
                                opacity: 0.6
                            }
                            QQC2.Label {
                                font.pointSize: referenceLabel.font.pointSize + 2
                                visible: releases.variant.realSize != releases.variant.size && releases.variant.realSize > 0.1
                                property double size: releases.variant.realSize
                                property string sizeString: size <= 0 ? "" :
                                                                        (size < 1024) ? (size + " B") :
                                                                                        (size < (1024 * 1024)) ? ((size / 1024).toFixed(1) + " KB") :
                                                                                                                 (size < (1024 * 1024 * 1024)) ? ((size / 1024 / 1024).toFixed(1) + " MB") :
                                                                                                                                                 ((size / 1024 / 1024 / 1024).toFixed(1) + " GB")
                                //: The downloaded image is compressed, this refers to the size which it will take after being decompressed and written to the drive
                                text: qsTr("(%1 after writing)").arg(sizeString)
                                opacity: 0.6
                            }
                        }
                        ColumnLayout {
                            width: parent.width
                            spacing: units.largeSpacing
                            opacity: releases.selected.isLocal ? 0.0 : 1.0
                            QQC2.Label {
                                font.pointSize: referenceLabel.font.pointSize + 1
                                visible: typeof releases.selected.version !== 'undefined'
                                text: releases.variant.name
                                opacity: 0.6
                            }
                            QQC2.Label {
                                font.pointSize: referenceLabel.font.pointSize - 1
                                visible: releases.selected.version && releases.variant
                                text: releases.variant.arch.details
                                opacity: 0.6
                            }
                            RowLayout {
                                spacing: 0
                                width: parent.width
                                QQC2.Label {
                                    font.pointSize: referenceLabel.font.pointSize - 1
                                    text: qsTr("Version %1").arg(releases.selected.version.name)
                                    color: versionMouse.containsPress ? Qt.lighter(palette.link, 1.5) : versionMouse.containsMouse ? Qt.darker(palette.link, 1.5) : palette.link
                                    opacity: versionRepeater.count <= 1 ? 0.6 : 1

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
                                            anchors.margins: -2
                                            visible: parent.activeFocus
                                        }
                                    }

                                    // TODO: Adwaita themed component
                                    QQC2.BusyIndicator {
                                        anchors.right: parent.left
                                        anchors.rightMargin: units.smallSpacing
                                        anchors.verticalCenter: parent.verticalCenter
                                        height: 24
                                        width: 24
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
                                        height: 1
                                    }

                                    AdwaitaPopOver {
                                        id: versionPopover
                                        z: 2
                                        anchors {
                                            horizontalCenter: parent.horizontalCenter
                                            top: parent.bottom
                                            topMargin: units.smallSpacing + opacity * units.gridUnit
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                prereleaseNotification.open = false
                                                archPopover.open = false
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: units.largeSpacing

                                            Repeater {
                                                id: versionRepeater
                                                model: releases.selected.versions
                                                QQC2.RadioButton {
                                                    text: name
                                                    Layout.alignment: Qt.AlignVCenter
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
                                            topMargin: units.largeSpacing + opacity * Math.round(units.gridUnit * 1.5)
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                versionPopover.open = false
                                                archPopover.open = false
                                            }
                                        }

                                        QQC2.Label {
                                            text: qsTr("Fedora %1 was released! Check it out!<br>If you want a stable, finished system, it's better to stay at version %2.").arg(releases.selected.prerelease).arg(releases.selected.version.name)
                                            font.pointSize: referenceLabel.font.pointSize - 1
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
                                QQC2.Label {
                                    // I'm sorry, everyone, I can't find a better way to determine if the date is valid
                                    visible: releases.selected.version.releaseDate.toLocaleDateString().length > 0
                                    text: qsTr(", released on %1").arg(releases.selected.version.releaseDate.toLocaleDateString())
                                    font.pointSize: referenceLabel.font.pointSize - 1
                                    opacity: 0.6
                                }
                                Item {
                                    Layout.fillWidth: true
                                }
                                QQC2.Label {
                                    Layout.alignment: Qt.AlignRight
                                    visible: releases.selected.version.variants.length > 1
                                    text: qsTr("Other variants...")
                                    font.pointSize: referenceLabel.font.pointSize - 1
                                    color: archMouse.containsPress ? Qt.lighter(palette.link, 1.5) : archMouse.containsMouse ? Qt.darker(palette.link, 1.5) : palette.link

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
                                            anchors.margins: -2
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
                                        height: 1
                                    }

                                    AdwaitaPopOver {
                                        id: archPopover
                                        z: 2
                                        anchors {
                                            horizontalCenter: parent.horizontalCenter
                                            top: parent.bottom
                                            topMargin: units.smallSpacing + opacity * units.gridUnit
                                        }

                                        onOpenChanged: {
                                            if (open) {
                                                versionPopover.open = false
                                                prereleaseNotification.open = false
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: units.largeSpacing

                                            Repeater {
                                                model: releases.selected.version.variants
                                                QQC2.RadioButton {
                                                    text: name
                                                    Layout.alignment: Qt.AlignVCenter
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
                QQC2.Label {
                    Layout.fillWidth: true
                    width: Layout.width
                    wrapMode: Text.WordWrap
                    text: releases.selected.description
                    textFormat: Text.RichText
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
    }
}
