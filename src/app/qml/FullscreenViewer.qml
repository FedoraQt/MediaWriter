/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
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
import QtQuick.Window 2.12

Item {
    id: fullscreenViewer
    anchors.fill: parent
    visible: false
    focus: true

    Keys.onEscapePressed: hide()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onClicked: fullscreenViewer.hide()
        cursorShape: Qt.PointingHandCursor
    }
    Rectangle {
        anchors.fill: parent
        color: fullscreenViewer.visible ? "black" : "transparent"
        Behavior on color { ColorAnimation { duration: 60 } }
    }

    function show(url) {
        fullscreenViewerImage.source = url
        fullscreenViewer.visible = true
        mainWindow.visibility = Window.FullScreen
        focus = true
    }
    function hide() {
        fullscreenViewerImage.source = ""
        fullscreenViewer.visible = false
        mainWindow.visibility = Window.Windowed
        focus = false
    }
    IndicatedImage {
        id: fullscreenViewerImage
        source: ""
        anchors.centerIn: parent
        property real sourceRatio: sourceSize.width / sourceSize.height
        property real screenRatio: Screen.width / Screen.height
        width: (sourceRatio > screenRatio ? Screen.width : Screen.height * sourceRatio)
        height: (sourceRatio < screenRatio ? Screen.height : Screen.width / sourceRatio)
    }
}
