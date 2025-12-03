import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    width: 800
    height: 600
    color: "transparent"

    property var contextIds: []
    property int contextLevel: 0

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text { id: header; text: contextIds.length > 0 ? "Transactions for " + contextIds[0] : "No date selected"; font.pointSize: 18 }

        ListView {
            id: txList
            model: 10
            delegate: Rectangle {
                height: 40
                width: parent.width
                color: index % 2 === 0 ? "#ffffff" : "#f7f7f7"
                Text { text: "Transaction " + index; anchors.centerIn: parent }
            }
        }
    }
}
