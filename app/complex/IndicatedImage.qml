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
import QtQuick.Controls 2.12

Item {
    id: root
    width: image.sourceSize.width
    height: image.sourceSize.height

    property alias cache: image.cache
    property alias sourceSize: image.sourceSize
    property alias fillMode: image.fillMode
    property alias source: image.source
    Image {
        id: image
        anchors.fill: parent
        smooth: true
        mipmap: true
        opacity: status == Image.Ready ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.OutQuad
                duration: 400
            }
        }
    }
    BusyIndicator {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: implicitWidth
        height: implicitHeight
        opacity: image.status == Image.Ready ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.InQuad
                duration: 100
            }
        }
    }
}
