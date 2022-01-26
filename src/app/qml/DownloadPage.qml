/*
 * Fedora Media Writer
 * Copyright (C) 2021 Evžen Gasta <evzen.ml@seznam.cz>
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

import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Window 6.2
import QtQuick.Layouts 6.2
import QtQml 6.2


Page {
    id: downloadPage
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        spacing: units.gridUnit
        
        Image {
            source: "qrc:/downloadPageImage"
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
        }
        
        Heading {
            Layout.alignment: Qt.AlignHCenter
            text: mainWindow.selectedOption == 1 ? qsTr("Writing %1").arg((String)(releases.localFile.iso).split("/").slice(-1)[0]) :                                                         qsTr("Downloading %1 ").arg(releases.selected.name)// + releases.selected.version.number
            level: 5
            Layout.preferredWidth: mainColumn.width
            elide: Label.ElideRight
            horizontalAlignment: Label.AlignHCenter
        }
        
        ColumnLayout {
            Label {
                Layout.alignment: Qt.AlignHCenter
                property double leftSize: releases.variant.progress.to - releases.variant.progress.value
                property string leftStr:  leftSize <= 0 ? "" : (leftSize < 1024) ? qsTr("(%1 B left)").arg(leftSize) : (leftSize < (1024 * 1024)) ? qsTr("(%1 KB left)").arg((leftSize / 1024).toFixed(1)) : (leftSize < (1024 * 1024 * 1024)) ? qsTr("(%1 MB left)").arg((leftSize / 1024 / 1024).toFixed(1)) :
                qsTr("(%1 GB left)").arg((leftSize / 1024 / 1024 / 1024).toFixed(1))
                text: releases.variant.statusString + (releases.variant.status == Units.Status.Downloading ? (" " + leftStr) : "")
            }
        
            ProgressBar {
                id: progressBar
                Layout.topMargin: units.gridUnit / 2
                Layout.fillWidth: true
                value: 0.0
            }
        }
        
        Column {
            id: infoColumn
            spacing: units.gridUnit / 2
            
            Label {
                id: messageDownload
                visible: false
                text: qsTr("The file will be saved to your Downloads folder.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageLoseData
                visible: false
                text: qsTr("By writing, you will lose all of the data on %1.").arg(drives.selected.name)
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageRestore
                visible: false
                text: qsTr("Your drive will be resized to a smaller capacity. You may resize it back to normal by using Fedora Media Writer. This will remove installation media from your drive.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageSelectedImage
                visible: releases.selected.isLocal
                text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.variant.iso ? (((String)(releases.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageArmBoard
                visible: false //boardCombo.otherSelected
                text: qsTr("Your board or device is not supported by Fedora Media Writer yet. Please check <a href=%1>this page</a> for more information about its compatibility with Fedora and how to create bootable media for it.").arg("https://fedoraproject.org/wiki/Architectures/ARM")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageDriveSize
                enabled: true
                visible: enabled && drives.selected && drives.selected.size > 160 * 1024 * 1024 * 1024 // warn when it's more than 160GB
                text: qsTr("The selected drive's size is %1. It's possible you have selected an external drive by accident!").arg(drives.selected ? drives.selected.readableSize : "N/A")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                visible: releases.variant && releases.variant.errorString.length > 0
                text: releases.variant ? releases.variant.errorString : ""
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
        }
    }
    
    states: [
        State {
            name: "preparing"
            when: releases.variant.status === Units.Status.Preparing
        },
        State {
            name: "downloading"
            when: releases.variant.status === Units.Status.Downloading
            PropertyChanges {
                target: messageDownload
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: releases.variant.progress.ratio
            }
        },
        State {
            name: "download_verifying"
            when: releases.variant.status === Units.Status.Downloading_Verifying
            PropertyChanges {
                target: messageDownload
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: releases.variant.progress.ratio;
            }
        },
        State {
            name: "ready_no_drives"
            when: releases.variant.status === Units.Status.Ready && drives.length <= 0
        },
        State {
            name: "ready"
            when: releases.variant.status === Units.Status.Ready && drives.length > 0
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        },
        State {
            name: "writing_not_possible"
            when: releases.variant.status === Units.Status.Writing_Not_Possible
        },
        State {
            name: "writing"
            when: releases.variant.status === Units.Status.Writing
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "write_verifying"
            when: releases.variant.status === Units.Status.Write_Verifying
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "finished"
            when: releases.variant.status === Units.Status.Finished
            PropertyChanges {
                target: messageDriveSize;
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: prevButton;
                text: qsTr("Back");
                visible: true
            }
            PropertyChanges {
                target: nextButton;
                visible: false
            }
            StateChangeScript {
                script: {
                    if (mainWindow.eraseVariant)
                        releases.variant.erase()
                }        
            }
        },
        State {
            name: "failed_verification_no_drives"
            when: releases.variant.status === Units.Status.Failed_Verification && drives.length <= 0
        },
        State {
            name: "failed_verification"
            when: releases.variant.status === Units.Status.Failed_Verification && drives.length > 0
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        },
        State {
            name: "failed_download"
            when: releases.variant.status === Units.Status.Failed_Download
        },
        State {
            name: "failed_no_drives"
            when: releases.variant.status === Units.Status.Failed && drives.length <= 0
        },
        State {
            name: "failed"
            when: releases.variant.status === Units.Status.Failed && drives.length > 0
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        }
    ]    
    
    CancelWindow {
        id: cancelWindow
    }
    
}

