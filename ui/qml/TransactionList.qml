import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    width: 800
    height: 600
    color: "transparent"

    // Simple grouped list: date header then transactions
    ListView {
        id: lv
        anchors.fill: parent
        model: transactionModel
        delegate: Column {
            spacing: 4
            Rectangle {
                width: parent.width
                height: 36
                color: "#efefef"
                Row {
                    anchors.fill: parent
                    anchors.margins: 4
                    Text { text: date; verticalAlignment: Text.AlignVCenter }
                    Spacer { }
                    ToolButton { text: "S"; onClicked: workspaceHost.onStatementActivated(date) }
                    ToolButton { text: "T"; onClicked: workspaceHost.onTransactionActivated(date) }
                }
            }
            // transactions under the date
            Repeater {
                model: transactions
                Rectangle { width: parent.width - 8; height: 40; color: "#ffffff"; radius: 4; anchors.horizontalCenter: parent.horizontalCenter
                    Row { anchors.fill: parent; anchors.margins: 8; spacing: 8
                        Text { text: modelData["label"] }
                        Spacer {}
                        Text { text: modelData["amount"] }
                    }
                }
            }
        }
    }
}
