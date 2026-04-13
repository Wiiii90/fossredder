import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    property var model: null

    function fileName(path) {
        if (!path) return ""
        const s = String(path).replace(/\\/g, "/")
        const idx = s.lastIndexOf("/")
        return idx >= 0 ? s.substring(idx + 1) : s
    }

    ListView {
        id: runsList
        anchors.fill: parent
        clip: true
        spacing: root.theme.spacingSmall
        model: root.model

        delegate: Rectangle {
            id: runEntry
            required property int index
            required property var time
            required property var status
            required property var file
            required property var path
            required property var message
            width: runsList.width
            color: "transparent"

            ColumnLayout {
                id: content
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: root.theme.margins * 2
                spacing: root.theme.margins * 2

                RowLayout {
                    Layout.fillWidth: true

                    Label { text: runEntry.time; font.pointSize: 10; opacity: 0.7; Layout.fillWidth: true; elide: Label.ElideRight }
                    Label { text: runEntry.status; font.pointSize: 10; color: runEntry.status === "Success" ? root.theme.success : (runEntry.status === "Running" ? root.theme.warning : root.theme.danger) }
                    Controls.Button { text: qsTr("Delete"); implicitHeight: 28; implicitWidth: 80; onClicked: { if (root.model && typeof root.model.remove === 'function') root.model.remove(runEntry.index, 1) } }
                }

                Label { text: (runEntry.file ? root.fileName(runEntry.file) : (runEntry.path ? root.fileName(runEntry.path) : "")); Layout.fillWidth: true; elide: Label.ElideRight }
                Label { visible: runEntry.message && runEntry.message.length > 0; text: runEntry.message; wrapMode: Text.WordWrap; opacity: 0.8; Layout.fillWidth: true }
            }

            implicitHeight: content.implicitHeight + root.theme.spacingMedium
        }
    }
}
