import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

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
        spacing: Theme.spacingSmall
        model: root.model

        delegate: Rectangle {
            width: runsList.width
            color: "transparent"

            ColumnLayout {
                id: content
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: Theme.margins * 2
                spacing: Theme.margins * 2

                RowLayout {
                    Layout.fillWidth: true

                    Label { text: time; font.pointSize: 10; opacity: 0.7; Layout.fillWidth: true; elide: Label.ElideRight }
                    Label { text: status; font.pointSize: 10; color: status === "Success" ? Theme.success : (status === "Running" ? Theme.warning : Theme.danger) }
                    Controls.Button { text: qsTr("Delete"); implicitHeight: 28; implicitWidth: 80; onClicked: { if (root.model && typeof root.model.remove === 'function') root.model.remove(index, 1) } }
                }

                Label { text: (file ? root.fileName(file) : (path ? root.fileName(path) : "")); Layout.fillWidth: true; elide: Label.ElideRight }
                Label { visible: message && message.length > 0; text: message; wrapMode: Text.WordWrap; opacity: 0.8; Layout.fillWidth: true }
            }

            implicitHeight: content.implicitHeight + Theme.spacingMedium
        }
    }
}
