import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: bar

    Rectangle {
        anchors.fill: parent
        color: "#f6fbfd"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8
            Label { text: (typeof statusText !== 'undefined' && statusText) ? statusText : qsTr("StatusBar"); color: "#213547" }
            Item { Layout.fillWidth: true }
            Label { text: (typeof statusItems !== 'undefined') ? (qsTr("Items: ") + statusItems) : qsTr("Items: 0"); color: "#213547" }
        }
    }
}
