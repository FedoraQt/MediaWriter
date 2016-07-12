import QtQuick 2.0

IndicatedImage {
    id: root
    property bool zoomed: false
    onZoomedChanged: {
        if (zoomed) {
            showAnimation.start()
        }
        else {
            hideAnimation.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.zoomed = !root.zoomed
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: root.zoomed ? 0.8 : 0.0
        Behavior on opacity { NumberAnimation { duration: 160 } }
    }

    Image {
        id: zoomedImage
        source: root.source
        visible: false
        SequentialAnimation {
            id: showAnimation
            onStarted: {
                zoomedImage.visible = true
            }

            ParallelAnimation {
                NumberAnimation {
                    target: zoomedImage; property: "width"
                    from: root.width
                    to: sourceSize.width
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "height"
                    from: root.height
                    to: sourceSize.height
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "x"
                    from: 0
                    to: (root.width - sourceSize.width) / 2
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "y"
                    from: 0
                    to: (root.height - sourceSize.height) / 2
                    easing.type: Easing.OutCubic
                }
            }
        }

        SequentialAnimation {
            id: hideAnimation
            onStopped: {
                zoomedImage.visible = false
            }

            ParallelAnimation {
                NumberAnimation {
                    target: zoomedImage; property: "width"
                    to: root.width
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "height"
                    to: root.height
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "x"
                    to: 0
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: zoomedImage; property: "y"
                    to: 0
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on height { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on width { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on x { NumberAnimation { easing.type: Easing.OutCubic } }
        Behavior on y { NumberAnimation { easing.type: Easing.OutCubic } }

        MouseArea {
            id: mouse
            enabled: root.zoomed
            anchors.fill: parent
            onClicked: root.zoomed = false
            onWheel: {
                if(wheel.angleDelta.y > 0 && zoomedImage.width < 10 * zoomedImage.sourceSize.width) {
                    zoomedImage.x -= zoomedImage.width * 0.15
                    zoomedImage.y -= zoomedImage.height * 0.15
                    zoomedImage.width *= 1.3
                    zoomedImage.height *= 1.3
                }
                else if (wheel.angleDelta.y < 0) {
                    if (zoomedImage.width * 0.7 < root.width) {
                        root.zoomed = false
                    }
                    else {
                        zoomedImage.x += zoomedImage.width * 0.15
                        zoomedImage.y += zoomedImage.height * 0.15
                        zoomedImage.width *= 0.7
                        zoomedImage.height *= 0.7
                    }
                }
            }
            drag {
                target: zoomedImage
                axis: Drag.XAndYAxis
                minimumX: - width + 32
                maximumX: (root.width - 32)
                minimumY: - height + 32
                maximumY: (root.height - 32)
            }
        }
    }

    Behavior on scale {
        NumberAnimation {
            easing.type: Easing.InOutElastic
        }
    }
}
