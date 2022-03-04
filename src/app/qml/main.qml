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

import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Window 6.2
import QtQuick.Layouts 6.2
import QtQml 6.2

ApplicationWindow {
    id: mainWindow
    visible: true
    minimumWidth: 640
    minimumHeight: 480
    
    property int selectedPage: Units.Page.MainPage
    property int selectedVersion: Units.Source.Product
    property int selectedOption: Units.MainSelect.Download
    
    property bool visibleCancelDialog: false
    property bool visibleAboutDialog: false
    property bool eraseVariant: false
    
    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        
        anchors.leftMargin: units.gridUnit * 3
        anchors.rightMargin: units.gridUnit * 3
        anchors.topMargin: units.gridUnit * 2
        anchors.bottomMargin: units.gridUnit * 2
    
        StackView {
            id: stackView
            initialItem: "MainPage.qml"
            
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: parent.width / 8
            Layout.rightMargin: parent.width / 8
            Layout.bottomMargin: units.gridUnit * 2
        }
        
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            
            Button {
                id: prevButton
                visible: selectedPage != Units.Page.DownloadPage
                text: getPrevButtonText()
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                enabled: mainLayout.state != "drivePage" 
                text: getNextButtonText()
            }
        }
        
        states: [
            State {
                name: "mainPage"
                when: selectedPage == Units.Page.MainPage
                PropertyChanges { 
                    target: mainWindow
                    title: qsTr("Fedora Media Writer") 
                }
                //When comming back from restore page, after successfull restoring a USB drive
                PropertyChanges { 
                    target: prevButton
                    text: getPrevButtonText()
                    onClicked: visibleAboutDialog = !visibleAboutDialog 
                }
                PropertyChanges { 
                    target: nextButton
                    enabled: true
                    onClicked: {
                        if (selectedOption == Units.MainSelect.Write)
                            selectedPage = Units.Page.DrivePage 
                        else if (selectedOption == Units.MainSelect.Restore)
                            selectedPage = Units.Page.RestorePage
                        else
                            selectedPage = Units.Page.VersionPage 
                    }
                }
                StateChangeScript {
                    script: {
                        //reset of source on versionPage
                        selectedOption = 0
                        releases.filterSource = 0
                        if (stackView.depth > 1) 
                            while(stackView.depth != 1)
                                stackView.pop() }
                }
            },
            State {
                name: "versionPage"
                when: selectedPage == Units.Page.VersionPage
                PropertyChanges { target: mainWindow; title: qsTr("Select Fedora Version") }
                PropertyChanges { target: nextButton; enabled: true; onClicked: selectedPage += 1 } 
                PropertyChanges { target: prevButton; onClicked: selectedPage -= 1 }
                StateChangeScript {
                    script: {
                        //state was pushing same page when returing from drivePage
                        if (stackView.depth <= 1)  
                            stackView.push("VersionPage.qml")
                    }
                }
            },
            State {
                name: "drivePage"
                when: selectedPage == Units.Page.DrivePage
                PropertyChanges { 
                    target: mainWindow
                    title: qsTr("Select drive") 
                }
                PropertyChanges {
                    target: nextButton;
                    enabled: true
                    onClicked: {
                        selectedPage = Units.Page.DownloadPage 
                        if (selectedOption != Units.MainSelect.Write) 
                            releases.variant.download()
                        drives.selected.setImage(releases.variant)
                        drives.selected.write(releases.variant)
                    }
                }
                PropertyChanges {
                    target: prevButton
                    onClicked: {
                        if (selectedOption == Units.MainSelect.Write)
                            selectedPage = Units.Page.MainPage
                        else {
                            selectedPage -= 1 
                            stackView.pop()
                        }
                    }
                }
                StateChangeScript { 
                    script: { stackView.push("DrivePage.qml") }
                }
            },
            State {
                name: "downloadPage"
                when: selectedPage == Units.Page.DownloadPage
                PropertyChanges {  
                    target: mainWindow
                    title: qsTr("Downloading") 
                }
                StateChangeScript {
                    script: { stackView.push("DownloadPage.qml") }
                }
                PropertyChanges {
                    target: prevButton;
                    onClicked: {
                        if (releases.variant.status != Units.DownloadStatus.Finished && releases.variant.status != Units.DownloadStatus.Failed && releases.variant.status != Units.DownloadStatus.Failed_Verification && releases.variant.status != Units.DownloadStatus.Failed_Download)
                            visibleCancelDialog = !visibleCancelDialog
                        drives.lastRestoreable = drives.selected
                        drives.lastRestoreable.setRestoreStatus(Units.RestoreStatus.Contains_Live)
                        releases.variant.resetStatus()
                        downloadManager.cancel()
                        selectedPage = Units.Page.MainPage
                    }
                }
                PropertyChanges {
                    target: nextButton;
                    enabled: true
                    onClicked: {
                        if (releases.variant.status === Units.DownloadStatus.Write_Verifying || releases.variant.status === Units.DownloadStatus.Writing || releases.variant.status === Units.DownloadStatus.Downloading)
                            visibleCancelDialog = !visibleCancelDialog
                        else {
                            releases.variant.resetStatus()
                            downloadManager.cancel()
                            selectedPage = Units.Page.MainPage
                        }
                    }
                }
            },
            State {
                name: "restorePage"
                when: selectedPage == Units.Page.RestorePage
                PropertyChanges { 
                    target: mainWindow
                    title: qsTr("Restore") 
                }
                PropertyChanges {
                    target: nextButton
                    enabled: true
                    onClicked: drives.lastRestoreable.restore() 
                }
                PropertyChanges {
                    target: prevButton
                    onClicked: selectedPage = Units.Page.MainPage 
                }
                StateChangeScript { 
                    script: { stackView.push("RestorePage.qml") }
                }
            }
        ]
    }
    
    Units {
        id: units
    }
    
    function getNextButtonText() {
        if (mainLayout.state == "restorePage") 
            return qsTr("Restore")
        else if (mainLayout.state == "downloadPage")
            return qsTr("Cancel")
        else if (mainLayout.state == "drivePage")
            return qsTr("Write")   
        return qsTr("Next")
    }
    
    function getPrevButtonText() {
        if (mainLayout.state == "mainPage") 
            return qsTr("About")
        return qsTr("Previous")
    }
}

