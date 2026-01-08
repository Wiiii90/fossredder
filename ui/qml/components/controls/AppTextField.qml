import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

TextField {
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: "#ddd"
        border.width: 1
    }
}
