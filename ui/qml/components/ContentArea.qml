import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: contentArea
    Layout.fillWidth: true
    Layout.fillHeight: true

    Rectangle {
        anchors.fill: parent
        color: "#fff8e1"

        Label {
            anchors.centerIn: parent
            text: qsTr("Content Area")
            color: "#213547"
            font.pointSize: 20
        }
    }
}
