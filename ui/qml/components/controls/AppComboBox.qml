import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

ComboBox {
    id: control
    Layout.fillWidth: true
    Layout.preferredHeight: 40
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: "#ddd"
        border.width: 1
    }
}
