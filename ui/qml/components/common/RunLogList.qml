import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import components.controls 1.0

Item {
    id: root
    property var model: null

    function fileName(path) {
        if (!path) return ""
        var s = String(path).replace(/\\/g, "/")
        var idx = s.lastIndexOf("/")
        return idx >= 0 ? s.substring(idx + 1) : s
    }

    ListView {
        id: runsList
        anchors.fill: parent
        clip: true
        spacing: 6
        model: root.model

        delegate: Rectangle {
            width: runsList.width
            color: "transparent"

            ColumnLayout {
                id: content
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 4
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true

                    Label { text: time; font.pointSize: 10; opacity: 0.7; Layout.fillWidth: true; elide: Label.ElideRight }
                    Label { text: status; font.pointSize: 10; color: status === "Success" ? "#1b7f1b" : (status === "Running" ? "#f1c40f" : "#a11") }
                    AppButton { text: qsTr("Delete"); implicitHeight: 28; implicitWidth: 80; onClicked: { if (root.model) root.model.removeAt(index) } }
                }

                Label { text: file ? root.fileName(file) : ""; Layout.fillWidth: true; elide: Label.ElideRight }
                Label { visible: message && message.length > 0; text: message; wrapMode: Text.WordWrap; opacity: 0.8; Layout.fillWidth: true }
            }

            implicitHeight: content.implicitHeight + 8
        }
    }
}
