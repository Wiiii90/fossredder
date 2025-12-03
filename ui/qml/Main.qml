import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "FossRedder QML POC"

    header: ToolBar {
        RowLayout {
            spacing: 8
            Button { text: "Import" }
            Button { text: "Export" }
            Button { text: "Settings" }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.topMargin: header ? header.height : 0
        model: []
        delegate: Rectangle {
            height: 48
            width: parent.width
            color: index % 2 == 0 ? "#ffffff" : "#f6f6f6"
            Text { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 12; text: modelData }
        }
    }
}
