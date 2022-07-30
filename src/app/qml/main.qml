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
    minimumWidth: Math.max(640, units.gridUnit * 32)
    maximumWidth: Math.max(640, units.gridUnit * 32)
    minimumHeight: Math.max(480, units.gridUnit * 25)
    maximumHeight: Math.max(480, units.gridUnit * 25)

    property int selectedPage: Units.Page.MainPage
    property int selectedVersion: Units.Source.Product
    property int selectedOption: Units.MainSelect.Download
    property QtObject lastRestoreable
    property bool eraseVariant: false
    property bool canOnlyDownloadFile: !drives.length && !downloadManager.isDownloaded(releases.selected.version.variant.url)
    
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
            
            pushEnter: Transition {
                XAnimator {
                    from: mainWindow.width
                    to: 0
                    duration: 250
                }
            }
            pushExit: Transition {
                XAnimator {
                    from: 0
                    to: -mainWindow.width
                    duration: 250
                }
            }
            popEnter: Transition {
              XAnimator {
                    from: -mainWindow.width
                    to: 0
                    duration: 250
                }
            }
            popExit: Transition {
                XAnimator {
                    from: 0
                    to: mainWindow.width
                    duration: 250
                }
            }
        }
        
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            
            Button {
                id: prevButton
                visible: true
                text: getPrevButtonText()
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                visible: mainLayout.state != "downloadPage" 
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
                    onClicked: aboutDialog.show()
                }
                PropertyChanges { 
                    target: nextButton
                    visible: true
                    onClicked: {
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
                StateChangeScript {
                    script: {
                        //reset of source on versionPage
                        selectedOption = Units.MainSelect.Download
                        releases.filterSource = 0
                        if (stackView.depth > 1)  {
                            while (stackView.depth != 1) {
                                stackView.pop()
                            }
                        }
                    }
                }
            },
            State {
                name: "versionPage"
                when: selectedPage == Units.Page.VersionPage
                PropertyChanges { target: mainWindow; title: qsTr("Select Fedora Version") }
                PropertyChanges { target: nextButton; visible: true; onClicked: selectedPage += 1 } 
                PropertyChanges { target: prevButton; visible: true; onClicked: selectedPage -= 1 }
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
                    title: qsTr("Select Drive") 
                }
                PropertyChanges {
                    target: nextButton;
                    visible: true
                    onClicked: {
                        selectedPage = Units.Page.DownloadPage 
                        if (selectedOption != Units.MainSelect.Write) 
                            releases.variant.download()
                        if (!canOnlyDownloadFile) {
                            selectedOption = Units.MainSelect.Download_And_Write
                            drives.selected.setImage(releases.variant)
                            drives.selected.write(releases.variant)
                        }
                    }
                }
                PropertyChanges {
                    target: prevButton
                    visible: true
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
                    script: { 
                        stackView.push("DrivePage.qml") 
                        eraseVariant = false
                    }
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
                    target: prevButton
                    visible: true
                    onClicked: {  
                        releases.variant.resetStatus()
                        downloadManager.cancel()
                        mainWindow.selectedPage = Units.Page.MainPage
                    }
                }
                PropertyChanges {
                    target: nextButton
                    onClicked: {
                        if (releases.variant.status === Units.DownloadStatus.Write_Verifying || releases.variant.status === Units.DownloadStatus.Writing || releases.variant.status === Units.DownloadStatus.Downloading || releases.variant.status === Units.DownloadStatus.Download_Verifying)
                            cancelDialog.show()
                        else if (releases.variant.status === Units.DownloadStatus.Write_Finished || releases.variant.status === Units.DownloadStatus.Download_Finished) {
                            drives.lastRestoreable = drives.selected
                            if (drives.lastRestoreable) 
                                drives.lastRestoreable.setRestoreStatus(Units.RestoreStatus.Contains_Live)
                            releases.variant.resetStatus()
                            downloadManager.cancel()
                            selectedPage = Units.Page.MainPage
                        } else if ((releases.variant.status === Units.DownloadStatus.Failed && drives.length > 0) || releases.variant.status === Units.DownloadStatus.Failed_Download || (releases.variant.status === Units.DownloadStatus.Failed_Verification && drives.length > 0) || releases.variant.status === Units.DownloadStatus.Ready) {
                            if (selectedOption != Units.MainSelect.Write)
                                releases.variant.download()
                            drives.selected.setImage(releases.variant)
                            drives.selected.write(releases.variant)
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
                    visible: true
                    onClicked: {
                        if (lastRestoreable && lastRestoreable.restoreStatus == Units.RestoreStatus.Restored)
                            selectedPage = Units.Page.MainPage 
                        else
                            drives.lastRestoreable.restore() 
                    }
                }
                PropertyChanges {
                    target: prevButton
                    visible: true
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
    
    AboutDialog {
        id: aboutDialog
    }
    
    CancelDialog {
        id: cancelDialog
    }
    
    
    function getNextButtonText() {
        if (mainLayout.state == "restorePage") {
            if (lastRestoreable && lastRestoreable.restoreStatus == Units.RestoreStatus.Restored)
                return qsTr("Finish")
            return qsTr("Restore")
        } else if (mainLayout.state == "drivePage") {
            if (selectedOption == Units.MainSelect.Write || downloadManager.isDownloaded(releases.selected.version.variant.url))
                return qsTr("Write")
            if (canOnlyDownloadFile) 
                return qsTr("Download")
            if (Qt.platform.os === "windows" || Qt.platform.os === "osx") 
                return qsTr("Download && Write")
            return qsTr("Download & Write") 
        } else if (mainLayout.state == "downloadPage") {
            if (releases.variant.status === Units.DownloadStatus.Write_Verifying || releases.variant.status === Units.DownloadStatus.Writing || releases.variant.status === Units.DownloadStatus.Downloading || releases.variant.status === Units.DownloadStatus.Download_Verifying)
                return qsTr("Cancel")
            else 
                return mainWindow.selectedOption == Units.MainSelect.Download ? qsTr("Write") : qsTr("Retry")
        }
        return qsTr("Next")
    }
    
    function getPrevButtonText() {
        if (mainLayout.state == "mainPage") 
            return qsTr("About")
        else if (mainLayout.state == "downloadPage")
            return qsTr("Cancel")
        return qsTr("Previous")
    }
}

