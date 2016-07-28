import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import "components"

ApplicationWindow {
    id: mainWindow
    visible: true
    minimumWidth: $(800)
    minimumHeight: $(480)
    title: qsTranslate("", "Fedora Media Writer")

    SystemPalette {
        id: palette
    }

    Component.onCompleted: {
        width = $(800)
        height = $(480)
    }

    property real scalingFactor: Math.ceil(Screen.pixelDensity * 25.4) / 96 > 1 ? Math.ceil(Screen.pixelDensity * 25.4) / 96 : 1
    function $(x) {
        return x * scalingFactor
    }

    property bool canGoBack: false
    property real margin: $(64) + (width - $(800)) / 4

    AdwaitaNotificationBar {
        id: deviceNotification
        text: open ? qsTranslate("", "You inserted <b>%1</b> that already contains a live system.<br>Do you want to restore it to factory settings?").arg(drives.lastRestoreable.name) : ""
        open: drives.lastRestoreable
        acceptText: qsTranslate("", "Restore")
        cancelText: qsTranslate("", "Do Nothing")
        property var disk: null
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        onAccepted: restoreDialog.visible = true

        Connections {
            target: drives
            onLastRestoreableChanged: {
                if (drives.lastRestoreable != null)
                    deviceNotification.open = true
            }
        }
    }

    Rectangle {
        anchors {
            top: deviceNotification.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        color: palette.window
        //radius: 8
        clip: true

        ListView {
            id: contentList
            anchors{
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            model: ["components/ImageList.qml", "components/ImageDetails.qml"]
            orientation: ListView.Horizontal
            snapMode: ListView.SnapToItem
            highlightFollowsCurrentItem: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            interactive: false
            highlightMoveVelocity: 3 * contentList.width
            highlightResizeDuration: 0
            cacheBuffer: 2*width
            delegate: Item {
                id: contentComponent
                width: contentList.width
                height: contentList.height
                Loader {
                    id: contentLoader
                    source: contentList.model[index]
                    anchors.fill: parent
                }
                Connections {
                    target: contentLoader.item
                    onStepForward: {
                        contentList.currentIndex++
                        canGoBack = true
                        releases.selectedIndex = index
                    }
                }
            }
        }
    }

    RestoreDialog {
        id: restoreDialog
    }
}

