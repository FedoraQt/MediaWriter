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

import QtQuick 6.6
import QtQuick.Controls 6.6 as QQC2
import QtQuick.Layouts 6.6

Page {
    id: versionPage
    property int prevSource: 0

    text: qsTr("Select Fedora Release")

    QQC2.Label {
        text: qsTr("Select from:")
    }

    QQC2.RadioButton {
        checked: true
        text: qsTr("Official Editions")
        onClicked: changeFilter(Units.Source.Product)
    }

    QQC2.RadioButton {
        text: qsTr("Atomic Desktops")
        onClicked: changeFilter(Units.Source.Emerging)
    }

    QQC2.RadioButton {
        text: qsTr("Spins")
        onClicked: changeFilter(Units.Source.Spins)
    }

    QQC2.RadioButton {
        text: qsTr("Labs")
        onClicked: changeFilter(Units.Source.Labs)
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

    onPreviousButtonClicked: selectedPage -= 1
    onNextButtonClicked: selectedPage += 1
}
