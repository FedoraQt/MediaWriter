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

QQC2.Page {
    default property alias content: layout.children

    property alias imageSource: image.source

    property alias text: heading.text
    property alias textLevel: heading.level

    property alias layoutSpacing: layout.spacing

    // Button.Text
    property alias previousButtonText: prevButton.text
    property alias nextButtonText: nextButton.text

    // Button.Visible
    property alias previousButtonVisible: prevButton.visible
    property alias nextButtonVisible: nextButton.visible

    // Button.Enabled
    property alias previousButtonEnabled: prevButton.enabled
    property alias nextButtonEnabled: nextButton.enabled

    // Button.Clicked
    signal previousButtonClicked()
    signal nextButtonClicked()

    Image {
        id: image
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        width: visible ? 280 : 0
        height: visible ? 210 : 0
        source: ""
        fillMode: Image.PreserveAspectFit
        sourceSize.width: parent.width
        sourceSize.height: parent.height
        smooth: true
        antialiasing: true
        visible: source != ""
    }

    ColumnLayout {
        id: mainLayout
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            top: image.bottom
            topMargin: units.gridUnit
        }
        spacing: units.gridUnit

        Heading {
            id: heading
            Layout.alignment: Qt.AlignHCenter
            level: 5
            visible: text
        }

        ColumnLayout {
            id: layout
            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: units.gridUnit * 4
            Layout.rightMargin: units.gridUnit * 4
        }

        RowLayout {
            id: buttonRow

            Layout.alignment: Qt.AlignBottom

            QQC2.Button {
                id: prevButton
                text: qsTr("Previous")
                onClicked: previousButtonClicked()
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: nextButton
                text: qsTr("Next")
                onClicked: nextButtonClicked();
            }
        }
    }
}
