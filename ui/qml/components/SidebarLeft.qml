import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: sidebarLeft

    Rectangle {
        anchors.fill: parent
        color: "#ffe5cc"

        Label {
            anchors.centerIn: parent
            text: qsTr("Sidebar Left")
            color: "#213547"
            font.pointSize: 16
        }
    }
}
