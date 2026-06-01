/*
 * AcreetionOS Media Writer
 * Copyright (C) 2026 Natalie <natalie@acreetionos.org>
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

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: units.gridUnit

        Image {
            id: image
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.preferredHeight: Math.min(210, parent.height * 0.4)
            Layout.maximumHeight: 210
            Layout.fillWidth: true
            source: ""
            fillMode: Image.PreserveAspectFit
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            smooth: true
            antialiasing: true
            visible: source != ""
        }

        Heading {
            id: heading
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.preferredWidth: mainLayout.width
            horizontalAlignment: Text.AlignHCenter
            level: 5
            maximumLineCount: 2
            visible: text
            wrapMode:Text.WordWrap
        }

        ColumnLayout {
            id: layout
            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 0
            Layout.leftMargin: units.gridUnit * 4
            Layout.rightMargin: units.gridUnit * 4
            clip: true
        }

        RowLayout {
            id: buttonRow
            Layout.alignment: Qt.AlignBottom
            Layout.topMargin: units.gridUnit

            QQC2.Button {
                id: prevButton
                text: qsTr("Previous")
                onClicked: previousButtonClicked()
                implicitWidth: Math.max(units.gridUnit * 6, textMetrics.width + units.gridUnit * 3)
                implicitHeight: units.gridUnit * 2
                background: Rectangle {
                    color: acreetionOSTheme.surfaceCard
                    border.color: acreetionOSTheme.green
                    border.width: 1
                    radius: 6
                }
                contentItem: Text {
                    id: prevBtnText
                    text: prevButton.text
                    color: acreetionOSTheme.green
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: Math.round(units.gridUnit * 0.6)
                }
                TextMetrics {
                    id: textMetrics
                    text: qsTr("Previous")
                    font.pixelSize: Math.round(units.gridUnit * 0.6)
                }
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: nextButton
                text: qsTr("Next")
                onClicked: nextButtonClicked();
                implicitWidth: Math.max(units.gridUnit * 6, nextMetrics.width + units.gridUnit * 3)
                implicitHeight: units.gridUnit * 2
                background: Rectangle {
                    color: nextButton.enabled ? acreetionOSTheme.buttonBackground(nextButton.enabled,
                        nextButton.hovered, nextButton.pressed) : acreetionOSTheme.surfaceDark
                    radius: 6
                }
                contentItem: Text {
                    text: nextButton.text
                    color: acreetionOSTheme.textOnAccent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: Math.round(units.gridUnit * 0.6)
                }
                TextMetrics {
                    id: nextMetrics
                    text: qsTr("Next")
                    font.pixelSize: Math.round(units.gridUnit * 0.6)
                }
            }
        }
    }
}
