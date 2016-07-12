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
        onClicked: root.zoomed = true
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
        //visible: height > parent.height || width > parent.width
        //x: (parent.width - width) / 2
        //y: (parent.height - height) / 2
        //width: root.zoomed ? sourceSize.width : parent.width
        //height: root.zoomed ? sourceSize.height : parent.height
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
                NumberAnimation {
                    target: zoomedImage; property: "scale"
                    to: 1.0
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on scale {
            NumberAnimation {
                easing.type: Easing.OutCubic
            }
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: root.zoomed = false
            onWheel: {
                if(wheel.angleDelta.y > 0 && parent.scale < 10)
                    parent.scale *= 1.5
                else if (wheel.angleDelta.y < 0) {
                    if (parent.scale / 1.5 < 1.0)
                        root.zoomed = false
                    else
                        parent.scale /= 1.5
                }
            }
            drag {
                target: parent
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
