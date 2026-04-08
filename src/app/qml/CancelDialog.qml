/*
 * Fedora Media Writer
 * Copyright (C) 2021-2022 Evžen Gasta <evzen.ml@seznam.cz>
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

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

ModalDialog {
    id: cancelDialog

    width: Math.max(360, units.gridUnit * 20)
    height: Math.max(180, units.gridUnit * 10)

    property QtObject drivesSelected
    property int variantStatus: releases.variant ? releases.variant.status : 0

    onOpened: drivesSelected = drives.selected

    contentItem: ColumnLayout {
        id: mainColumn
        spacing: units.gridUnit

        Column {
            leftPadding: units.gridUnit
            rightPadding: units.gridUnit
            spacing: units.gridUnit

            Heading {
                level: 2
                text: {
                    if (variantStatus === Units.DownloadStatus.Downloading || variantStatus === Units.DownloadStatus.Download_Verifying)
                        qsTr("Cancel Download?")
                    else if (variantStatus === Units.DownloadStatus.Writing)
                        qsTr("Cancel Writing?")
                    else
                        qsTr("Cancel Verification?")
                }
            }

            QQC2.Label {
                text: {
                    if (variantStatus === Units.DownloadStatus.Downloading || variantStatus === Units.DownloadStatus.Download_Verifying)
                        qsTr("Download and media writing will be aborted. This process can be resumed any time later.")
                    else if (variantStatus === Units.DownloadStatus.Writing)
                        qsTr("Writing process will be aborted and your drive will have to be restored afterwards.")
                    else
                        qsTr("This operation is safe to be cancelled.")
                }
                wrapMode: QQC2.Label.Wrap
                width: mainColumn.width - units.gridUnit * 2
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignBottom

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: continueButton
                onClicked: cancelDialog.close()
                text: qsTr("Continue")
            }

            QQC2.Button {
                id: cancelButton
                onClicked: {
                    cancelDialog.close()
                    // Store release state locally as drives.selected.cancel() makes
                    // it go to failed state if we cancel the writing process
                    if (drives.selected)
                        drives.selected.cancel()
                    if (mainWindow.selectedPage === Units.Page.DownloadPage &&
                        (variantStatus === Units.DownloadStatus.Writing || variantStatus === Units.DownloadStatus.Write_Verifying || variantStatus === Units.DownloadStatus.Writing_Not_Possible)) {
                        drivesSelected.setRestoreStatus(Units.RestoreStatus.Contains_Live)
                    }
                    releases.variant.resetStatus()
                    downloadManager.cancel()
                    selectedPage = Units.Page.MainPage
                }
                text: {
                    if (variantStatus === Units.DownloadStatus.Downloading || variantStatus === Units.DownloadStatus.Download_Verifying)
                        qsTr("Cancel Download")
                    else if (variantStatus === Units.DownloadStatus.Writing)
                        qsTr("Cancel Writing")
                    else if (variantStatus === Units.DownloadStatus.Write_Verifying)
                        qsTr("Cancel Verification")
                    else
                        qsTr("Cancel")
                }
            }
        }
    }
}
