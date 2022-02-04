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
    id: mainWindow
    visible: true
    minimumWidth: 640
    minimumHeight: 480
    
    property int selectedPage: Units.Page.MainPage
    property int selectedVersion: Units.Source.Product
    property int selectedOption: Units.MainSelect.Download
    
    property bool enNextButton: true
    property bool visibleCancelWindow: false
    property bool visibleAboutWindow: false
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
                onClicked: {
                    stackView.pop()
                    
                    if (selectedPage == Units.Page.DownloadPage) {
                        if (releases.variant.status != Units.DownloadStatus.Finished && releases.variant.status != Units.DownloadStatus.Failed && releases.variant.status != Units.DownloadStatus.Failed_Verification && releases.variant.status != Units.DownloadStatus.Failed_Download)
                            visibleCancelWindow = !visibleCancelWindow
                        selectedPage = Units.Page.MainPage
                        releases.variant.resetStatus()
                        downloadManager.cancel()
                    }
                    else if (selectedPage == Units.Page.MainPage)
                        visibleAboutWindow = !visibleAboutWindow
                    else if (selectedOption == Units.MainSelect.Write || selectedOption == Units.MainSelect.Restore)
                        selectedPage = Units.Page.MainPage
                    else 
                        selectedPage -= 1
                    
                    selectedOption = 0
                }
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                visible: true
                enabled: mainLayout.state != "drivePage" && mainLayout.state != "restorePage"? true : enNextButton
                text: getNextButtonText()
                onClicked: {
                    if (mainLayout.state == "mainPage") {
                        //reset of source on versionPage
                        releases.filterSource = 0
                        selectedPage = selectedOption == Units.MainSelect.Write ? Units.Page.DrivePage : selectedOption == Units.MainSelect.Restore ? Units.Page.RestorePage : Units.Page.VersionPage
                    }
                    else if (mainLayout.state == "drivePage") {
                        selectedPage = Units.Page.DownloadPage 
                        if (selectedOption != Units.MainSelect.Write) 
                            releases.variant.download()
                        drives.selected.setImage(releases.variant)
                        drives.selected.write(releases.variant)
                    }
                    else if (mainLayout.state == "restorePage")
                        drives.lastRestoreable.restore()  
                    else if (mainLayout.state == "downloadPage") {
                        if (releases.variant.status === Units.DownloadStatus.Write_Verifying || releases.variant.status === Units.DownloadStatus.Writing || releases.variant.status === Units.DownloadStatus.Downloading)
                            visibleCancelWindow = !visibleCancelWindow
                        else {
                            //drives.selected.cancel()
                            releases.variant.resetStatus()
                            downloadManager.cancel()
                            selectedPage = Units.Page.MainPage
                        }
                    }
                    else
                        selectedPage += 1
                }
            }
        }
        
        states: [
            State {
                name: "mainPage"
                when: selectedPage == Units.Page.MainPage
                PropertyChanges { target: mainWindow; title: qsTr("Fedora Media Writer") }
                StateChangeScript {
                    script: {
                        if (stackView.depth > 1) 
                            while(stackView.depth != 1)
                                stackView.pop()
                    }
                }
            },
            State {
                name: "versionPage"
                when: selectedPage == Units.Page.VersionPage
                PropertyChanges { target: mainWindow; title: qsTr("Select Fedora Version") }
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
                PropertyChanges { target: mainWindow; title: qsTr("Select drive") }
                StateChangeScript {
                    script: stackView.push("DrivePage.qml")
                }
            },
            State {
                name: "downloadPage"
                when: selectedPage == Units.Page.DownloadPage
                PropertyChanges { target: mainWindow; title: qsTr("Downloading") }
                StateChangeScript {
                    script: stackView.push("DownloadPage.qml")
                }
            },
            State {
                name: "restorePage"
                when: selectedPage == Units.Page.RestorePage
                PropertyChanges { target: mainWindow; title: qsTr("Restore") }
                StateChangeScript {
                    script: stackView.push("RestorePage.qml")
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

