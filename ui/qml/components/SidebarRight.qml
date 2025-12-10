import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: sidebarRight

    Rectangle {
        anchors.fill: parent
        color: "#e1f7ff"

        Label {
            anchors.centerIn: parent
            text: qsTr("Sidebar Right")
            color: "#213547"
            font.pointSize: 16
        }
    }
}
