import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Controls.Private 1.0

ComboBox {
    implicitWidth: $(128)
    implicitHeight: $(32)
    style: ComboBoxStyle {
        background: AdwaitaRectangle {
            width: control.width
            Arrow {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: $(16)
                scale: $(1.2)
                rotation: 90
            }
        }
        font.pixelSize: $(12)
        label: Text {
            width: control.width
            x: $(4)
            font.pixelSize: $(12)
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            text: control.currentText
        }
    }
}

