import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: bar

    Rectangle {
        anchors.fill: parent
        color: "#f6fbfd"
        border.color: "#e3eef6"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            Label {
                id: lblStatus
                text: (typeof statusText !== 'undefined' && statusText) ? statusText : qsTr("Bereit")
                color: "#213547"
                elide: Label.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
