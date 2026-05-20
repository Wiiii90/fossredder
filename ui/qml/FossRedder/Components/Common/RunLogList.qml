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
