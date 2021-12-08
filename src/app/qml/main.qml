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
    
    ColumnLayout {
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
                onClicked: {
                    if (selectedPage == Units.Page.MainPage) {
                        stackView.push("AboutPage.qml") 
                        selectedPage = Units.Page.AboutPage
                    }
                    else if (selectedPage == Units.Page.VersionPage) {
                        stackView.pop("VersionPage.qml")
                        selectedPage = Units.Page.MainPage
                    }
                    else if (selectedPage == Units.Page.DrivePage) {
                        stackView.pop("DrivePage.qml")
                        selectedPage = selectISO ? Units.Page.MainPage : Units.Page.VersionPage 
                        selectISO = false
                        enNextButton = true
                    }
                    else if (selectedPage == Units.Page.AboutPage) {
                        stackView.pop("AboutPage.qml")
                        selectedPage = Units.Page.MainPage
                    }
                    else if (selectedPage == Units.Page.RestorePage) {
                        stackView.pop("RestorePage.qml")
                        selectedPage = Units.Page.MainPage
                        restoreDrive = false
                    }
                }
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                enabled: enNextButton
                onClicked: {
                    if (selectedPage == Units.Page.MainPage) {
                        if (restoreDrive) {
                            stackView.push("RestorePage.qml")
                            selectedPage = Units.Page.RestorePage
                        }
                        else if (selectISO) {
                            stackView.push("DrivePage.qml")
                            selectedPage = Units.Page.DrivePage
                        }
                        else {
                            stackView.push("VersionPage.qml")
                            selectedPage = Units.Page.VersionPage
                        }
                    }
                    else if (selectedPage == Units.Page.VersionPage) {
                        stackView.push("DrivePage.qml")
                        selectedPage = Units.Page.DrivePage
                    }
                    else if (selectedPage == Units.Page.DrivePage) {
                        stackView.push("DownloadPage.qml")
                        onClicked: drives.selected.write(releases.variant)
                        selectedPage = Units.Page.DownloadPage
                    }
                    else if (selectedPage == Units.Page.DownloadPage) {
                        stackView.pop("DownloadPage.qml")
                        selectedPage = Units.Page.DrivePage
                    }
                    else if (selectedPage == Units.Page.RestorePage) {
                        drives.lastRestoreable.restore()
                    }
                }
            }
        }
        
        states: [
            State {
                name: "aboutPage"
                when: selectedPage == Units.Page.AboutPage
                PropertyChanges { target: mainWindow; title: qsTr("About") }
                PropertyChanges { target: prevButton; text: qsTr("Previous") }
                PropertyChanges { target: nextButton; visible: false }
            },
            State {
                name: "mainPage"
                when: selectedPage == Units.Page.MainPage
                PropertyChanges { target: mainWindow; title: qsTr("Fedora Media Writer") }
                PropertyChanges { target: prevButton; text: qsTr("About") }
                PropertyChanges { target: nextButton; text: qsTr("Next"); visible: true }
            },
            State {
                name: "versionPage"
                when: selectedPage == Units.Page.VersionPage
                PropertyChanges { target: mainWindow; title: qsTr("Select Fedora Version") }
                PropertyChanges { target: prevButton; text: qsTr("Previous") }
                PropertyChanges { target: nextButton; text: qsTr("Next") }
            },
            State {
                name: "drivePageISOSelected"
                when: selectedPage == Units.Page.DrivePage && selectISO
                PropertyChanges { target: mainWindow; title: qsTr("Select file and drive") }
                PropertyChanges { target: prevButton; text: qsTr("Previous") }
                PropertyChanges { target: nextButton; text: qsTr("Write") }
            },
            State {
                name: "drivePage"
                when: selectedPage == Units.Page.DrivePage && !selectISO
                PropertyChanges { target: mainWindow; title: qsTr("Select drive") }
                PropertyChanges { target: prevButton; text: qsTr("Previous") }
                PropertyChanges { target: nextButton; text: qsTr("Download") }
            },
            State {
                name: "downloadPage"
                when: selectedPage == Units.Page.DownloadPage
                PropertyChanges { target: mainWindow; title: qsTr("Downloading") }
                PropertyChanges { target: prevButton; visible: false }
                PropertyChanges { target: nextButton; text: qsTr("Cancel") }
            },
            State {
                name: "restorePage"
                when: selectedPage == Units.Page.RestorePage
                PropertyChanges { target: mainWindow; title: qsTr("Restore") }
                PropertyChanges { target: prevButton; text: qsTr("Previous") }
                PropertyChanges { target: nextButton; text: qsTr("Restore") }
            }
        ]
    }
    
    Units {
        id: units
    }
}

