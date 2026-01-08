import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Button {
    id: control
    property alias label: text
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    background: Rectangle {
        color: control.down ? Theme.primary.darker(120) : Theme.primary
        radius: Theme.radius
        border.width: 0
    }

    contentItem: Text {
        text: control.text
        color: Theme.surface
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
