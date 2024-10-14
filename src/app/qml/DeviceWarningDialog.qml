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

import QtQuick 6.6
import QtQuick.Controls 6.6
import QtQuick.Window 6.6
import QtQuick.Layouts 6.6

ApplicationWindow {
    id: cancelDialog

    minimumWidth: Math.max(360, units.gridUnit * 20)
    maximumWidth: Math.max(360, units.gridUnit * 20)
    minimumHeight: Math.max(180, units.gridUnit * 10)
    maximumHeight: Math.max(180, units.gridUnit * 10)

    modality: Qt.ApplicationModal
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    title: " "

    ColumnLayout {
        id: mainColumn
        anchors {
            fill: parent
            margins: units.gridUnit
        }
        spacing: units.gridUnit

        Heading {
            level: 2
            Layout.fillWidth: true
            text: "Erase confirmation"
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("The device you have selected appears to be larger than 32GB. Are you sure you want to completely erase all the data on it?")
            wrapMode: Label.Wrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignBottom
            Layout.fillHeight: true

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: cancelButton
                onClicked: cancelDialog.close()
                text: qsTr("Cancel")
            }

            Button {
                id: continueButton
                onClicked: {
                    cancelDialog.close()
                    selectedPage = Units.Page.DownloadPage
                    drives.selected.setImage(releases.variant)
                    drives.selected.write(releases.variant)
                }
                text: {
                    if (selectedOption == Units.MainSelect.Write || downloadManager.isDownloaded(releases.selected.version.variant.url))
                        return qsTr("Write")
                    if (Qt.platform.os === "windows" || Qt.platform.os === "osx")
                        return qsTr("Download && Write")
                    return qsTr("Download & Write")
                }
            }
        }
    }
}

