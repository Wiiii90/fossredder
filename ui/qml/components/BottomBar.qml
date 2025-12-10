import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: bottomBar

    Rectangle {
        anchors.fill: parent
        color: "#ffdede"

        Label {
            anchors.centerIn: parent
            text: qsTr("Bottom Bar")
            color: "#213547"
            font.pointSize: 16
        }
    }
}
