/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Components/Common/RunLogList.qml
 * @brief Provides the RunLogList component.
 */

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

    function payloadSummary(payloadText) {
        if (!payloadText || String(payloadText).length === 0) return ""
        try {
            const payload = JSON.parse(String(payloadText))
            const items = payload && payload.items && payload.items.length !== undefined ? payload.items : []
            if (!items || items.length === 0) return ""

            let annualCount = 0
            let analysisCount = 0
            const exportTypes = []
            for (let i = 0; i < items.length; ++i) {
                const item = items[i]
                if (!item || !item.objectType) continue
                const type = String(item.objectType).toLowerCase()
                if (type === "annual") annualCount += 1
                if (type === "analysis") {
                    analysisCount += 1
                    if (item.exportType && exportTypes.indexOf(item.exportType) < 0) exportTypes.push(item.exportType)
                }
            }

            const parts = []
            if (annualCount > 0) parts.push(qsTr("Annuals: %1").arg(annualCount))
            if (analysisCount > 0) parts.push(qsTr("Analyses: %1").arg(analysisCount))
            if (exportTypes.length > 0) parts.push(qsTr("Formats: %1").arg(exportTypes.join(", ")))
            return parts.join(" | ")
        } catch (e) {
            return ""
        }
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
            property var payload: ""
            property bool draftAttached: false
            property string draftId: ""
            property string statementId: ""
            width: runsList.width
            radius: root.theme.radius
            color: "transparent"
            border.width: root.theme.borderWidthThin
            border.color: hoverArea.containsMouse ? root.theme.accent : root.theme.border

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: runEntry.draftAttached || (runEntry.statementId && runEntry.statementId.length > 0) || !runEntry.draftAttached
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
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize

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

                    Controls.SecondaryButton {
                        text: "×"
                        implicitHeight: root.theme.viewCompactActionButtonSize
                        implicitWidth: root.theme.viewCompactActionButtonSize
                        textColor: root.theme.textMuted
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

                Label {
                    visible: root.payloadSummary(runEntry.payload).length > 0
                    text: root.payloadSummary(runEntry.payload)
                    wrapMode: Text.WordWrap
                    opacity: 0.8
                    color: root.theme.textMuted
                    Layout.fillWidth: true
                }
            }

            implicitHeight: content.implicitHeight + root.theme.spacingSmall
        }
    }
}
