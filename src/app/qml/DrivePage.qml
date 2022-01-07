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
import QtQuick.Dialogs 6.2

Page {
    ColumnLayout {
        anchors.fill: parent
        spacing: units.gridUnit
        
        Heading {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Write Options")
            level: 5
        }
        
        ColumnLayout {
            id: architectureCol
            visible: !selectedOption == 1
            Heading {
                text: qsTr("Hardware Architecture")
            }
            
            ComboBox {
                id: hwArchCombo
                Layout.fillWidth: true
                model: releases.selected.version.variants
                textRole: "name"
                onCurrentIndexChanged: releases.selected.version.variantIndex = currentIndex
            }
        }
        
        ColumnLayout {
            visible: selectedOption == 1
            Heading {
                text: qsTr("Selected file")
            }
            
            RowLayout {
                id: fileCol
                Label {
                    text: releases.localFile.iso ? (String)(releases.localFile.iso).split("/").slice(-1)[0] : ("<font color=\"gray\">" + qsTr("None") + "</font>")
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
                Button {
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("Select ...")
                    onClicked: {
                        if (portalFileDialog.isAvailable)
                            portalFileDialog.open()
                        else
                            fileDialog.open()
                    }

                    Connections {
                        target: portalFileDialog
                        function onFileSelected(fileName) {
                            releases.selectLocalFile(fileName)
                        }
                    }
                    
                    FileDialog {
                        id: fileDialog
                        nameFilters: [ qsTr("Image files") + " (*.iso *.raw *.xz)", qsTr("All files (*)")]
                        onAccepted: {
                            releases.selectLocalFile(fileUrl)
                        }
                    }
    
                }
            }
        }
        
        ColumnLayout {
            Heading {
                text: qsTr("USB Drive")
            }
            
            ComboBox {
                id: driveCombo
                Layout.fillWidth: true
                model: drives
                enabled: !(currentIndex === -1 || !currentText)
                displayText: currentIndex === -1 || !currentText ? qsTr("There are no portable drives connected") : currentText
                textRole: "display"
                
                Binding on currentIndex {
                    when: drives
                    value: drives.selectedIndex
                }
                Binding {
                    target: drives
                    property: "selectedIndex"
                    value: driveCombo.currentIndex
                }
            }
        }
        
        ColumnLayout {
            Layout.bottomMargin: units.gridUnit * 5
            
            Heading {
                text: qsTr("Download")
                level: 1
            }
        
            CheckBox {
                text: qsTr("Delete download after writing")
            }
        }
    }
    
    states: [
        State {
            name: "ISONotSelected"
            when: !selectedOption == 1
            PropertyChanges { target: mainWindow; enNextButton: driveCombo.enabled && hwArchCombo.currentIndex + 1 }
            StateChangeScript { script: releases.setSelectedVariantIndex = 0 }
        },
        State {
            name: "ISOSelected"
            when: selectedOption == 1
            PropertyChanges { target: mainWindow; enNextButton: driveCombo.enabled && releases.localFile.iso }
        }
    ]
}
