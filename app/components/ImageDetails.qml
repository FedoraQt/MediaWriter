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

Item {
    id: root
    anchors.fill: parent

    signal stepForward

    ScrollView {
        anchors {
            fill: parent
            leftMargin: anchors.rightMargin
        }
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
                        onClicked: {
                            archPopover.open = false
                            versionPopover.open = false
                            canGoBack = false
                            contentList.currentIndex--
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    AdwaitaButton {
                        text: qsTr("Create Live USB")
                        color: "#628fcf"
                        textColor: "white"
                        onClicked: {
                            archPopover.open = false
                            versionPopover.open = false
                            dlDialog.visible = true
                            releases.selected.version.variant.download()
                        }
                        enabled: !releases.selected.isLocal || releases.selected.version.variant.iso
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
                                font.pixelSize: $(17)
                                text: releases.selected.name
                            }
                            Text {
                                anchors.right: parent.right
                                font.pixelSize: $(15)
                                property double size: releases.selected.version.variant.size
                                text: size <= 0 ? "" :
                                      (size < 1024) ? (size + " B") :
                                      (size < (1024 * 1024)) ? ((size / 1024).toFixed(1) + " KB") :
                                      (size < (1024 * 1024 * 1024)) ? ((size / 1024 / 1024).toFixed(1) + " MB") :
                                      ((size / 1024 / 1024 / 1024).toFixed(1) + " GB")

                                color: "gray"
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                            height: childrenRect.height

                            ColumnLayout {
                                width: parent.width
                                spacing: $(6)
                                opacity: releases.selected.isLocal ? 0.0 : 1.0
                                Text {
                                    font.pixelSize: $(13)
                                    color: "gray"
                                    visible: typeof releases.selected.version !== 'undefined'
                                    text: releases.selected.version.variant.arch.description
                                }
                                Text {
                                    font.pixelSize: $(11)
                                    color: "gray"
                                    visible: releases.selected.version && releases.selected.version.variant
                                    text: releases.selected.version.variant.arch.details
                                }
                                RowLayout {
                                    spacing: 0
                                    width: parent.width
                                    Text {
                                        text: qsTr("Version %1").arg(releases.selected.version.number)
                                        font.pixelSize: $(11)

                                        color: versionRepeater.count <= 1 ? "gray" : versionMouse.containsPress ? "#284875" : versionMouse.containsMouse ? "#447BC7" : "#315FA0"
                                        Behavior on color { ColorAnimation { duration: 100 } }
                                        MouseArea {
                                            id: versionMouse
                                            enabled: versionRepeater.count > 1
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                versionPopover.open = !versionPopover.open
                                            }
                                        }

                                        Rectangle {
                                            visible: versionRepeater.count > 1
                                            anchors {
                                                left: parent.left
                                                right: parent.right
                                                top: parent.bottom
                                                topMargin: $(1)
                                            }
                                            radius: height / 2
                                            color: parent.color
                                            height: $(1.2)
                                        }


                                        PopOver {
                                            id: versionPopover
                                            anchors {
                                                horizontalCenter: parent.horizontalCenter
                                                top: parent.bottom
                                                topMargin: $(8) + opacity * $(24)
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
                                    }
                                    Text {
                                        // I'm sorry, everyone, I can't find a better way to determine if the date is valid
                                        visible: releases.selected.version.releaseDate.toLocaleDateString().length > 0
                                        text: qsTr(", released on %1").arg(releases.selected.version.releaseDate.toLocaleDateString())
                                        font.pixelSize: $(11)
                                        color: "gray"
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        Layout.alignment: Qt.AlignRight
                                        //visible: releases.selected.arch.length
                                        text: qsTr("Other variants...")
                                        font.pixelSize: $(11)
                                        color: archMouse.containsPress ? "#284875" : archMouse.containsMouse ? "#447BC7" : "#315FA0"
                                        Behavior on color { ColorAnimation { duration: 100 } }
                                        MouseArea {
                                            id: archMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                if (versionPopover.open)
                                                    versionPopover.open = false
                                                else
                                                    archPopover.open = !archPopover.open
                                            }
                                        }

                                        Rectangle {
                                            anchors {
                                                left: parent.left
                                                right: parent.right
                                                top: parent.bottom
                                                topMargin: $(1)
                                            }
                                            radius: height / 2
                                            color: parent.color
                                            height: $(1.2)
                                        }

                                        PopOver {
                                            id: archPopover
                                            anchors {
                                                horizontalCenter: parent.horizontalCenter
                                                top: parent.bottom
                                                topMargin: $(8) + opacity * $(24)
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

                            RowLayout {
                                id: localSelectionLayout
                                visible: releases.selected.isLocal
                                spacing: $(12)
                                AdwaitaButton {
                                    text: qsTr("Select Live ISO")
                                    onClicked: {
                                        fileDialog.visible = false // for some reason it got stuck in the closed state once in a while, so ensure it's actually closed
                                        fileDialog.visible = true
                                    }
                                }
                                Text {
                                    font.pixelSize: $(12)
                                    text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.selected.version.variant.iso ? (((String)(releases.selected.version.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
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
                    font.pixelSize: $(12)
                }
                Repeater {
                    id: screenshotRepeater
                    model: releases.selected.screenshots
                    ZoomableImage {
                        z: 1
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
                color: "#dddddd"
                implicitWidth: $(11)
                implicitHeight: $(11)
            }
            handle: Rectangle {
                color: "#b3b5b6"
                x: $(2)
                y: $(2)
                implicitWidth: $(7)
                implicitHeight: $(7)
                radius: $(4)
            }
            transientScrollBars: false
            handleOverlap: $(1)
            minimumHandleLength: $(10)
        }
    }

    DownloadDialog {
        id: dlDialog
        onVisibleChanged: {
            //if (!visible)
            //    releases.selected.
        }
    }
    FileDialog {
        id: fileDialog
        nameFilters: [ qsTr("Image files (*.iso)"), qsTr("All files (*)")]
        onAccepted: {
            releases.setLocalFile(fileUrl)
        }
    }
}
