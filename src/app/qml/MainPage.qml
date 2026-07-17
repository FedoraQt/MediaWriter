/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

Page {
    id: mainPage

    imageSource: "qrc:/mainPageImage"
    text: qsTr("Select Image Source")

    QQC2.RadioButton {
        id: downloadRadio
        checked: selectedOption == Units.MainSelect.Download
        text: mainWindow.mnemonic(qsTr("<u>D</u>ownload automatically"))
        onClicked: {
            selectedOption = Units.MainSelect.Download
        }
        Shortcut {
            sequence: "Alt+D"
            enabled: mainPage.visible
            onActivated: {
                downloadRadio.checked = true
                downloadRadio.clicked()
            }
        }
    }

    QQC2.RadioButton {
        id: adoptIsoRadio
        text: mainWindow.mnemonic(qsTr("<u>S</u>elect .iso file"))
        onClicked: {
            selectedOption = Units.MainSelect.Write
            releases.selectLocalFile("")
        }
        Shortcut {
            sequence: "Alt+S"
            enabled: mainPage.visible
            onActivated: {
                adoptIsoRadio.checked = true
                adoptIsoRadio.clicked()
            }
        }
    }

    QQC2.RadioButton {
        id: restoreRadio
        visible: drives.restoreableDrives.length > 0
        text: mainWindow.mnemonic(drives.restoreableDrives.length === 1 ? qsTr("<u>R</u>estore <b>%1</b>").arg(drives.restoreableDrives[0].name) : qsTr("<u>R</u>estore a USB drive (%1 available)").arg(drives.restoreableDrives.length))
        onClicked: {
            selectedOption = Units.MainSelect.Restore
        }
        Shortcut {
            sequence: "Alt+R"
            enabled: mainPage.visible && restoreRadio.visible
            onActivated: {
                restoreRadio.checked = true
                restoreRadio.clicked()
            }
        }
    }

    // HACK: enforces all the items above to move up and make smaller
    // space between the image and the heading
    Item {
        Layout.fillHeight: true
    }

    previousButtonText: mainWindow.mnemonic(qsTr("<u>A</u>bout"))
    previousButtonShortcut: "Alt+A"

    onPreviousButtonClicked: {
        aboutDialog.open()
    }

    onNextButtonClicked: {
        if (selectedOption == Units.MainSelect.Write) {
            if (releases.localFile.iso)
                releases.selectLocalFile()
            selectedPage = Units.Page.DrivePage
        } else if (selectedOption == Units.MainSelect.Restore)
            selectedPage = Units.Page.RestorePage
        else
            selectedPage = Units.Page.VersionPage
    }
}
