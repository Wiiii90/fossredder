import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    property var model: null
    signal runClicked(int index, string logId, bool draftAttached, string statementId)
    signal deleteClicked(int index, bool draftAttached, string draftId)

    function friendlyDateTime(value) {
        if (!value) return ""
        const raw = String(value)
        const d = new Date(raw)
        if (!isNaN(d.getTime())) return Qt.formatDateTime(d, Qt.DefaultLocaleShortDate)
        return raw
    }

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
        topMargin: root.theme.spacingSmall
        spacing: root.theme.spacingSmall
        model: root.model

        delegate: Rectangle {
            id: runEntry
            required property int index
            required property string logId
            required property var time
            required property var status
            required property var file
            required property var message
            required property bool draftAttached
            required property string draftId
            required property string statementId
            width: runsList.width
            radius: root.theme.radius
            color: "transparent"
            border.width: root.theme.borderWidthThin
            border.color: hoverArea.containsMouse ? root.theme.accent : root.theme.border

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: runEntry.draftAttached || (runEntry.statementId && runEntry.statementId.length > 0)
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: root.runClicked(runEntry.index, runEntry.logId, runEntry.draftAttached, runEntry.statementId)
            }

            ColumnLayout {
                id: content
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: root.theme.spacingSmall
                anchors.topMargin: root.theme.spacingSmall
                anchors.rightMargin: root.theme.spacingSmall
                anchors.bottomMargin: root.theme.spacingSmall
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24

                    Label {
                        text: root.friendlyDateTime(runEntry.time)
                        font.pointSize: 10
                        opacity: 0.7
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Label.ElideRight
                    }
                    Label {
                        text: runEntry.status
                        font.pointSize: 10
                        Layout.alignment: Qt.AlignVCenter
                        verticalAlignment: Text.AlignVCenter
                        color: runEntry.status === "Success" || runEntry.status === "Finalized"
                               ? root.theme.success
                               : (runEntry.status === "Running" || runEntry.status === "Draft"
                                   ? root.theme.warning
                                   : root.theme.danger)
                    }

                    Controls.Button {
                        text: "×"
                        implicitHeight: 22
                        implicitWidth: 24
                        fillColor: root.theme.surface
                        textColor: root.theme.textMuted
                        bordered: true
                        onClicked: root.deleteClicked(runEntry.index, runEntry.draftAttached, runEntry.draftId)
                    }
                }

                Label {
                    text: runEntry.file ? root.fileName(runEntry.file) : ""
                    Layout.fillWidth: true
                    elide: Label.ElideRight
                }

                Label {
                    visible: runEntry.message && runEntry.message.length > 0
                    text: runEntry.message
                    wrapMode: Text.WordWrap
                    opacity: 0.8
                    Layout.fillWidth: true
                }
            }

            implicitHeight: content.implicitHeight + root.theme.spacingSmall
        }
    }
}
