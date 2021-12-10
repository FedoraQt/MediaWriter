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
    property bool selectISO: false 
    property bool restoreDrive: false
    
    property bool enNextButton: true
    property bool enPrevButton: true
    property bool visibPrevButton: true
    
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
                visible: mainLayout.state != "downloadPage"
                text: getPrevButtonText()
                onClicked: {
                    stackView.pop()
                    selectedPage = selectISO || restoreDrive || mainLayout.state == "aboutPage"? Units.Page.MainPage : selectedPage - 1
                    selectISO = false
                    restoreDrive = false
                }
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                visible: mainLayout.state != "aboutPage"
                enabled: mainLayout.state != "drivePage" && mainLayout.state != "restorePage"? true : enNextButton
                text: getNextButtonText()
                onClicked: {
                    if (mainLayout.state == "mainPage" && selectISO)
                        selectedPage = Units.Page.DrivePage
                    else if (mainLayout.state == "mainPage" && restoreDrive)
                        selectedPage = Units.Page.RestorePage
                    else if (mainLayout.state == "drivePage" && selectISO)
                    {
                        selectedPage += 1
                        drives.selected.write(releases.localFile)
                    }
                    else if (mainLayout.state == "drivePage" && !selectISO)
                    {
                        releases.variant.download()
                        // TODO start download
                    }
                    else if (mainLayout.state == "restorePage")
                        drives.lastRestoreable.restore()  
                    else if (mainLayout.state == "downloadPage")
                        selectedPage = Units.Page.MainPage
                    else
                        selectedPage += 1
                    //selectedPage = selectISO ? Units.Page.DrivePage : restoreDrive ? Units.Page.RestorePage : selectedPage + 1
                }
            }
        }
        
        states: [
            State {
                name: "aboutPage"
                when: selectedPage == Units.Page.AboutPage
                PropertyChanges { target: mainWindow; title: qsTr("About") }
                StateChangeScript {
                    script: stackView.push("AboutPage.qml")
                }
            },
            State {
                name: "mainPage"
                when: selectedPage == Units.Page.MainPage
                PropertyChanges { target: mainWindow; title: qsTr("Fedora Media Writer") }
                StateChangeScript {
                    script: {
                        if (stackView.depth > 1) 
                            while(stackView.depth != 1)
                                stackView.pop()
                        else 
                            stackView.push("MainPage.qml")
                    }
                }
            },
            State {
                name: "versionPage"
                when: selectedPage == Units.Page.VersionPage
                PropertyChanges { target: mainWindow; title: qsTr("Select Fedora Version") }
                StateChangeScript {
                    script: stackView.push("VersionPage.qml")
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

