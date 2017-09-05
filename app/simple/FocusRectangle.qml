import QtQuick 2.0

// TODO use a BorderImage or something else to draw a dashed rectangle, this is ugly
BorderImage {
    border {
        left: 2
        right: 2
        top: 2
        bottom: 2
    }
    source: "qrc:/focusRect"
    horizontalTileMode: BorderImage.Repeat
    verticalTileMode: BorderImage.Repeat
}
