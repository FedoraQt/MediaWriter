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
    id: versionPage
    property int prevSource: 0

    text: qsTr("Select Fedora Release")

    QQC2.Label {
        text: qsTr("Select from:")
    }

    QQC2.RadioButton {
        id: officialRadio
        checked: true
        text: mainWindow.mnemonic(qsTr("<u>O</u>fficial Editions"))
        onClicked: changeFilter(Units.Source.Product)
        Shortcut {
            sequence: "Alt+O"
            enabled: versionPage.visible
            onActivated: {
                officialRadio.checked = true
                officialRadio.clicked()
            }
        }
    }

    QQC2.RadioButton {
        id: atomicosRadio
        text: mainWindow.mnemonic(qsTr("<u>A</u>tomic Desktops"))
        onClicked: changeFilter(Units.Source.Emerging)
        Shortcut {
            sequence: "Alt+A"
            enabled: versionPage.visible
            onActivated: {
                atomicosRadio.checked = true
                atomicosRadio.clicked()
            }
        }
    }

    QQC2.RadioButton {
        id: fedospinRadio
        text: mainWindow.mnemonic(qsTr("<u>S</u>pins"))
        onClicked: changeFilter(Units.Source.Spins)
        Shortcut {
            sequence: "Alt+S"
            enabled: versionPage.visible
            onActivated: {
                fedospinRadio.checked = true
                fedospinRadio.clicked()
            }
        }
    }

    QQC2.RadioButton {
        id: fedolabsRadio
        text: mainWindow.mnemonic(qsTr("<u>L</u>abs"))
        onClicked: changeFilter(Units.Source.Labs)
        Shortcut {
            sequence: "Alt+L"
            enabled: versionPage.visible
            onActivated: {
                fedolabsRadio.checked = true
                fedolabsRadio.clicked()
            }
        }
    }

    QQC2.ComboBox {
        id: selectFromComboBox
        Layout.fillWidth: true
        Layout.topMargin: units.gridUnit / 2
        textRole: "name"
        valueRole: "sourceIndex"
        model: releases
        onCurrentValueChanged: updateSelectedIndex()
    }

    function changeFilter(filter) {
        releases.filterSource = filter
        if (releases.filterSource != prevSource) {
            prevSource = releases.filterSource
            selectFromComboBox.currentIndex = 0
        }
    }

    function updateSelectedIndex() {
        // Guard passing an invalid value we get when resetting
        // index while changing filter above
        if (selectFromComboBox.currentValue) {
            releases.selectedIndex = parseInt(selectFromComboBox.currentValue)
        }
    }

    onPreviousButtonClicked: selectedPage = Units.Page.MainPage
    onNextButtonClicked: selectedPage = Units.Page.DrivePage
}
