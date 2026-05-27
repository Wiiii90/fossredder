/**
 * @file ui/qml/FossRedder/Components/Common/RunLogList.qml
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
    property string selectedLogId: ""
    property int cardMinHeight: 0
    property real cardRadius: -1
    property int cardPadding: root.theme.spacingSmall
    property int listTopMargin: root.theme.spacingSmall
    property int itemSpacing: root.theme.spacingSmall
    property color baseBorderColor: root.theme.border
    property color hoverBorderColor: root.theme.accent
    property int actionButtonSize: root.theme.viewCompactActionButtonSize
    property int actionButtonTopInset: 0
    property int actionButtonRightInset: 0
    property int headerTopInset: 0
    signal runClicked(int index, string logId, bool draftAttached, string statementId, string draftId)
    signal deleteClicked(int index, bool draftAttached, string draftId)

    function friendlyDateTime(value) {
        if (!value) return ""
        const raw = String(value)
        const d = new Date(raw)
        if (!isNaN(d.getTime())) return Qt.formatDateTime(d)
        return raw
    }

    function fileName(path) {
        if (!path) return ""
        const s = String(path).replace(/\\/g, "/")
        const idx = s.lastIndexOf("/")
        return idx >= 0 ? s.substring(idx + 1) : s
    }

    function isExportPayload(payloadText) {
        if (!payloadText || String(payloadText).length === 0) return false
        try {
            const payload = JSON.parse(String(payloadText))
            return !!(payload && payload.items && payload.items.length !== undefined)
        } catch (e) {
            return false
        }
    }

    function exportTitleFromPayload(payloadText) {
        if (!payloadText || String(payloadText).length === 0) return ""
        try {
            const payload = JSON.parse(String(payloadText))
            const items = payload && payload.items && payload.items.length !== undefined ? payload.items : []
            if (!items || items.length === 0) return qsTr("Export")

            const analysisNames = []
            for (let i = 0; i < items.length; ++i) {
                const item = items[i]
                if (!item || String(item.objectType).toLowerCase() !== "analysis") continue
                const name = item.objectName ? String(item.objectName).trim() : ""
                if (name.length > 0) analysisNames.push(name)
            }

            if (analysisNames.length === 0) return qsTr("Export")
            if (analysisNames.length === 1) return qsTr("Export '%1'").arg(analysisNames[0])
            const quotedNames = analysisNames.map(function(name) { return "'" + name + "'" })
            return qsTr("Export %1").arg(quotedNames.join(", "))
        } catch (e) {
            return ""
        }
    }

    function titleText(file, payloadText) {
        if (root.isExportPayload(payloadText)) {
            const exportTitle = root.exportTitleFromPayload(payloadText)
            if (exportTitle.length > 0) return exportTitle
        }
        return file ? root.fileName(file) : ""
    }

    function exportStatusFallback(status) {
        const normalized = String(status || "").toLowerCase()
        if (normalized === "success") return qsTr("Export completed successfully.")
        if (normalized === "running") return qsTr("Starting export...")
        return ""
    }

    function statusDetailText(message, status, payloadText) {
        const messageText = message ? String(message).trim() : ""
        if (messageText.length > 0) return messageText
        if (root.isExportPayload(payloadText)) return root.exportStatusFallback(status)
        return ""
    }

    readonly property int detailLineHeight: detailLineMetric.height

    TextMetrics {
        id: detailLineMetric
        text: "Mg"
    }

    function selectionColor() {
        return root.theme && root.theme.selectionHighlight !== undefined
            ? root.theme.selectionHighlight
            : root.theme.accent
    }

    ListView {
        id: runsList
        objectName: "runLogList"
        anchors.fill: parent
        clip: true
        topMargin: root.listTopMargin
        spacing: root.itemSpacing
        model: root.model

        delegate: Rectangle {
            id: runEntry
            objectName: "runLogCard_" + logId
            required property int index
            required property string logId
            required property var time
            required property var status
            required property var file
            required property var message
            property var payload: ""
            required property bool draftAttached
            required property string draftId
            required property string statementId
            readonly property string normalizedStatus: String(status || "").toLowerCase()
            readonly property bool navigableDraft: draftAttached || draftId.length > 0 || normalizedStatus === "draft"
            readonly property bool navigableStatement: statementId.length > 0
            readonly property bool selected: root.selectedLogId.length > 0
                                             && (root.selectedLogId === runEntry.logId
                                                 || root.selectedLogId === runEntry.draftId)
            width: runsList.width
            height: Math.max(root.cardMinHeight, content.implicitHeight + root.cardPadding)
            radius: root.cardRadius >= 0 ? root.cardRadius : root.theme.radius
            color: "transparent"
            border.width: root.theme.borderWidthThin
            border.color: runEntry.selected
                          ? root.selectionColor()
                          : (rowClickArea.containsMouse ? root.hoverBorderColor : root.baseBorderColor)

            ColumnLayout {
                id: content
                z: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: root.cardPadding
                anchors.topMargin: root.cardPadding
                anchors.rightMargin: root.cardPadding
                anchors.bottomMargin: root.cardPadding
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.actionButtonSize + root.actionButtonTopInset
                    Layout.topMargin: root.headerTopInset

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
                        color: runEntry.normalizedStatus === "success" || runEntry.normalizedStatus === "finalized"
                               ? root.theme.success
                               : (runEntry.normalizedStatus === "running" || runEntry.normalizedStatus === "paused" || runEntry.normalizedStatus === "draft"
                                   ? root.theme.warning
                                   : root.theme.danger)
                    }

                    Controls.SecondaryButton {
                        z: 3
                        objectName: "runLogDelete_" + runEntry.logId
                        text: "×"
                        implicitHeight: root.actionButtonSize
                        implicitWidth: root.actionButtonSize
                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                        Layout.topMargin: root.actionButtonTopInset
                        Layout.rightMargin: root.actionButtonRightInset
                        textColor: root.theme.textMuted
                        onClicked: root.deleteClicked(runEntry.index, runEntry.draftAttached, runEntry.draftId)
                    }
                }

                Label {
                    text: root.titleText(runEntry.file, runEntry.payload)
                    Layout.fillWidth: true
                    elide: Label.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.detailLineHeight
                    Layout.minimumHeight: root.detailLineHeight
                    text: root.statusDetailText(runEntry.message, runEntry.status, runEntry.payload)
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    opacity: text.length > 0 ? 0.8 : 0
                    color: root.theme.textMuted
                }
            }

            MouseArea {
                id: rowClickArea
                objectName: "runLogRow_" + runEntry.logId
                z: 2
                anchors.fill: parent
                anchors.rightMargin: root.actionButtonSize + root.theme.spacingSmall + root.actionButtonRightInset
                hoverEnabled: true
                preventStealing: true
                enabled: runEntry.navigableDraft || runEntry.navigableStatement
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                function activateRun() {
                    root.runClicked(runEntry.index, runEntry.logId, runEntry.navigableDraft, runEntry.statementId, runEntry.draftId)
                }
                onClicked: activateRun()
            }

            implicitHeight: height
        }
    }
}
