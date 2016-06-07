import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Button {
    id: root
    property color color: palette.button
    property color textColor: palette.buttonText

    style: ButtonStyle {
        background: AdwaitaRectangle {
            color: root.color
        }
        label: Item {
            implicitWidth: labelText.width + $(16)
            Text {
                x: $(8)
                font.pixelSize: $(12)
                id: labelText
                color: control.enabled ? root.textColor : "gray"
                text: control.text
                height: parent.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}

