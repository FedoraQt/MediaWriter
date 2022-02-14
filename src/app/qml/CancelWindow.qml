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

ApplicationWindow {
    id: cancelWindow
    visible: mainWindow.visibleCancelWindow
    minimumWidth: mainWindow.width / 2 + units.gridUnit * 10
    minimumHeight: mainWindow.height / 3
    modality: Qt.ApplicationModal
    
    Component.onCompleted: {
        width = mainWindow.minimumWidth / 2 + units.gridUnit * 10
        height = mainWindow.minimumHeight / 3
    }
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        anchors.leftMargin: units.gridUnit * 3
        anchors.rightMargin: units.gridUnit * 3
        anchors.topMargin: units.gridUnit * 2
        anchors.bottomMargin: units.gridUnit * 2
        spacing: units.gridUnit
           
        Column {
            spacing: units.gridUnit
            
            Label {
                Layout.alignment: Qt.AlignVCenter
                text: releases.variant.status == Units.DownloadStatus.Downloading ? qsTr("Downloading is still in progress, do you wish to cancel it?") : qsTr("Do you want to cancel writing?")
                wrapMode: Label.Wrap
                width: mainColumn.width
                horizontalAlignment: Label.AlignHCenter
            }
            
            //Label {
                //Layout.alignment: Qt.AlignVCenter
                //text: releases.variant.status == Units.DownloadStatus.Writing ? qsTr("random text?") : qsTr("other random text")
                //wrapMode: Label.Wrap
                //width: mainColumn.width
                //horizontalAlignment: Label.AlignHCenter
            //}
        }
          
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: continueButton
                onClicked: mainWindow.visibleCancelWindow = !mainWindow.visibleCancelWindow
                text: qsTr("Cancel")
            }
            
            Button {
                id: cancelButton
                onClicked: {
                    mainWindow.visibleCancelWindow = !mainWindow.visibleCancelWindow
                    //TODO is correct to unselect drive?
                    //drives.selected.cancel()
                    releases.variant.resetStatus()
                    downloadManager.cancel()
                    mainWindow.selectedPage = Units.Page.MainPage
                }
                text: qsTr("Ok")
            }
        }
    }
}

