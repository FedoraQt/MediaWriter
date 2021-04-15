/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 * Copyright (C) 2020 Jan Grulich <jgrulich@redhat.com>
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

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

QQC2.Button {
    id: deleteButton

    property string errorText: ""
    signal started

    enabled: true
    visible: opacity > 0.0
    opacity: 1.0

    QQC2.BusyIndicator {
        id: indicator
        anchors.centerIn: parent
        implicitHeight: deleteButton.height
        implicitWidth: deleteButton.width
        opacity: 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation {
                duration: 120
            }
        }
    }

    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: deleteButton
                opacity: 0.0
            }
        },
        State {
            name: "ready"
            PropertyChanges {
                target: deleteButton
                enabled: true
                text: qsTr("Delete the Downloaded Image")
            }
            PropertyChanges {
                target: deleteButton
                highlighted: true
            }
        },
        State {
            name: "working"
            PropertyChanges {
                target: indicator
                opacity: 1.0
            }
            StateChangeScript {
                name: "colorChange"
                script: if (deleteButton.hasOwnProperty("destructiveAction")) {
                            deleteButton.destructiveAction = false
                        }
            }
        },
        State {
            name: "success"
            PropertyChanges {
                target: deleteButton
                // TODO
                icon.name: "qrc:/icons/dialog-information"
            }
            PropertyChanges {
                target: hideTimer
                running: true
            }
        },
        State {
            name: "error"
            PropertyChanges {
                target: deleteButton
                // TODO
                icon.name: "qrc:/icons/dialog-error"
            }
            PropertyChanges {
                target: deleteButton
                text: deleteButton.errorText
            }
        }
    ]

    Timer {
        id: workTimer
        interval: 800
        onTriggered: {
            started()
        }
    }

    Timer {
        id: hideTimer
        interval: 5000
        onTriggered: {
            deleteButton.state = "hidden"
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 240
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: state == "working" ? 120 : 240
        }
    }

    onClicked: {
        deleteButton.state = "working"
        workTimer.start()
    }

    Component.onCompleted: {
        if (deleteButton.hasOwnProperty("destructiveAction")) {
            deleteButton.destructiveAction = true
        }
    }
}
