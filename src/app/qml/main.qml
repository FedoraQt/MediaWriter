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

import QtQuick 6.6
import QtQuick.Controls 6.6

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
    
    StackView {
        id: stackView
        initialItem: "MainPage.qml"

        anchors {
            fill: parent
            margins: units.gridUnit * 2
        }

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
        
        states: [
            State {
                name: "mainPage"
                when: selectedPage == Units.Page.MainPage
                PropertyChanges { 
                    target: mainWindow
                    title: qsTr("Fedora Media Writer") 
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
                    title: releases.variant.statusString
                }
                StateChangeScript {
                    script: { stackView.push("DownloadPage.qml") }
                }
            },
            State {
                name: "restorePage"
                when: selectedPage == Units.Page.RestorePage
                PropertyChanges {
                    target: mainWindow
                    title: qsTr("Restore")
                }
                StateChangeScript {
                    script: { stackView.push("RestorePage.qml") }
                }
            }
        ]
    }
    
    Connections {
        target: drives
        function onLastRestoreableChanged() {
            if (!drives.selected)
                selectedPage = Units.Page.MainPage
        }
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
}

