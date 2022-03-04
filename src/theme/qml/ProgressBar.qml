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

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import QtQuick.Templates as T
import org.fedoraproject.AdwaitaTheme 

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    property color progressBarColor: theme.highlightColor

    contentItem: Item {
        scale: control.mirrored ? -1 : 1

        Rectangle {
            id: indicator
            height: parent.height
            width: control.indeterminate ? theme.progressBarBusyIndicatorSize : parent.width * control.position
            color: control.progressBarColor
            border.color: theme.getProgressBarOutlineColor()
        }

        SequentialAnimation {
            id: animation
            loops: Animation.Infinite

            alwaysRunToEnd: true
            running: control.indeterminate && control.visible

            PropertyAnimation {
                target: indicator
                property: "x"
                duration: 2500
                from: 1
                to: control.width - indicator.width
            }
            PropertyAnimation {
                target: indicator
                property: "x"
                duration: 2500
                to: 1
            }
        }
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: theme.progressBarThickness + 0.5
        color: theme.getProgressBarColor()
        border.color: theme.getProgressBarOutlineColor()
        radius: 0.5
    }

}
