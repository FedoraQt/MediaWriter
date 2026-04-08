/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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
    id: deviceWarningDialog

    width: Math.max(360, units.gridUnit * 20)
    height: Math.max(180, units.gridUnit * 10)

    contentItem: ColumnLayout {
        id: mainColumn
        spacing: units.gridUnit

        Heading {
            level: 2
            Layout.fillWidth: true
            text: qsTr("Erase confirmation")
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: qsTr("The entire device (all of %1) will be erased and the selected image will be written to it. Do you want to continue?").arg(formatSize(drives.selected ? drives.selected.size : 0))
            wrapMode: QQC2.Label.Wrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignBottom
            Layout.fillHeight: true

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: cancelButton
                onClicked: deviceWarningDialog.close()
                text: qsTr("Cancel")
            }

            QQC2.Button {
                id: continueButton
                onClicked: {
                    deviceWarningDialog.close()
                    selectedPage = Units.Page.DownloadPage
                    if (drives.selected) {
                        drives.selected.setImage(releases.variant)
                        drives.selected.write(releases.variant)
                    }
                }
                text: {
                    const variant = releases.selected && releases.selected.version ? releases.selected.version.variant : null
                    if (selectedOption === Units.MainSelect.Write || (variant && downloadManager.isDownloaded(variant.url)))
                        return qsTr("Write")
                    if (Qt.platform.os === "windows" || Qt.platform.os === "osx")
                        return qsTr("Download && Write")
                    return qsTr("Download & Write")
                }
            }
        }
    }

    function formatSize(bytes) {
        const kb = 1024;
        const mb = kb * 1024;
        const gb = mb * 1024;

        if (bytes >= gb) {
            return (bytes / gb).toFixed(2) + " " + qsTr("GB");
        } else if (bytes >= mb) {
            return (bytes / mb).toFixed(2) + " " + qsTr("MB");
        } else if (bytes >= kb) {
            return (bytes / kb).toFixed(2) + " " + qsTr("KB");
        } else {
            return bytes + " " + qsTr("Bytes");
        }
    }
}
