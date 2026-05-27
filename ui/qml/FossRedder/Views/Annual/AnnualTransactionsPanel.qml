/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualTransactionsPanel.qml
 * @brief Provides the AnnualTransactionsPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property var transactions: []
    property var groupedTransactions: ({ deduplicated: [], similar: [], divergent: [], workspaceOnly: [] })
    property int minTableWidth: 720

    function amountText(value) {
        const amount = Number(value)
        if (isNaN(amount))
            return "0.00"
        return amount.toFixed(2)
    }

    function asArray(value) {
        return value && value.length !== undefined ? value : []
    }

    function sourceNames(row) {
        if (!row || !row.sourceAnalysisNames || row.sourceAnalysisNames.length === 0)
            return ""
        return String(row.sourceAnalysisNames.join(", "))
    }

    function statusColor(status) {
        const s = Number(status || 0)
        if (s === 3)
            return root.theme.success ? root.theme.success : root.theme.textPrimary
        if (s === 2)
            return root.theme.info ? root.theme.info : root.theme.textPrimary
        if (s === 1)
            return root.theme.warning ? root.theme.warning : root.theme.textPrimary
        return root.theme.textPrimary
    }

    property var sectionExpanded: ({
        deduplicated: true,
        similar: true,
        divergent: true,
        workspaceOnly: true,
        missingLive: true
    })

    function isExpanded(key) {
        return !!(root.sectionExpanded && root.sectionExpanded[key])
    }

    function toggleExpanded(key) {
        var next = {
            deduplicated: root.isExpanded("deduplicated"),
            similar: root.isExpanded("similar"),
            divergent: root.isExpanded("divergent"),
            workspaceOnly: root.isExpanded("workspaceOnly"),
            missingLive: root.isExpanded("missingLive")
        }
        next[key] = !next[key]
        root.sectionExpanded = next
    }

    function missingLiveRows() {
        const groups = root.groupedTransactions || ({})
        const merged = []
        const seen = ({})
        const buckets = [
            root.asArray(groups.deduplicated),
            root.asArray(groups.similar),
            root.asArray(groups.divergent)
        ]
        for (let b = 0; b < buckets.length; ++b) {
            const rows = buckets[b]
            for (let i = 0; i < rows.length; ++i) {
                const row = rows[i] || ({})
                if (!row.isMissingLive)
                    continue
                const key = String(row.key || row.id || ("missingLive-" + b + "-" + i))
                if (seen[key])
                    continue
                seen[key] = true
                merged.push(row)
            }
        }
        return merged
    }

    function groupedCount() {
        const groups = root.groupedTransactions || ({})
        return root.asArray(groups.deduplicated).length
                + root.asArray(groups.similar).length
                + root.asArray(groups.divergent).length
                + root.asArray(groups.workspaceOnly).length
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Annual transactions")
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius
            color: root.theme.surfaceAlt
            border.width: 1
            border.color: root.theme.border

            Flickable {
                id: txScroll
                anchors.fill: parent
                anchors.margins: root.theme.spacingSmall
                clip: true
                contentWidth: txRows.width
                contentHeight: txRows.implicitHeight

                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                Column {
                    id: txRows
                    width: Math.max(txScroll.width, root.minTableWidth)
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: [
                            { key: "deduplicated", title: qsTr("Included entries (exact matches)"), rows: root.asArray(root.groupedTransactions ? root.groupedTransactions.deduplicated : []) },
                            { key: "similar", title: qsTr("Included entries (possible variants)"), rows: root.asArray(root.groupedTransactions ? root.groupedTransactions.similar : []) },
                            { key: "divergent", title: qsTr("Included entries (unique)"), rows: root.asArray(root.groupedTransactions ? root.groupedTransactions.divergent : []) },
                            { key: "workspaceOnly", title: qsTr("Missing live transactions from selected year"), rows: root.asArray(root.groupedTransactions ? root.groupedTransactions.workspaceOnly : []) },
                            { key: "missingLive", title: qsTr("Included deleted transactions"), rows: root.missingLiveRows() }
                        ]

                        delegate: Column {
                            required property var modelData
                            width: txRows.width
                            spacing: root.theme.spacingSmall
                            visible: modelData.rows.length > 0

                            Rectangle {
                                objectName: "annualTransactionsSectionToggle_" + String(modelData.key)
                                width: parent.width
                                height: root.theme.controlHeight
                                radius: root.theme.radius
                                color: root.theme.surface
                                border.width: 1
                                border.color: root.theme.border

                                Label {
                                    anchors.fill: parent
                                    anchors.leftMargin: root.theme.spacingSmall
                                    anchors.rightMargin: root.theme.spacingSmall
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    text: (root.isExpanded(modelData.key) ? "▾ " : "▸ ")
                                          + modelData.title + " (" + modelData.rows.length + ")"
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.toggleExpanded(modelData.key)
                                }
                            }

                            Repeater {
                                model: root.isExpanded(modelData.key) ? modelData.rows : []

                                delegate: Rectangle {
                                    id: txRow
                                    required property var modelData
                                    width: parent ? parent.width : txRows.width
                                    implicitHeight: rowLayout.implicitHeight + (root.theme.spacingSmall * 2)
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: 1
                                    border.color: txRow.modelData && txRow.modelData.isMixedYear
                                                  ? root.theme.danger
                                                  : root.theme.border

                                    RowLayout {
                                        id: rowLayout
                                        anchors.fill: parent
                                        anchors.leftMargin: root.theme.spacingSmall
                                        anchors.rightMargin: root.theme.spacingSmall
                                        anchors.topMargin: root.theme.spacingSmall
                                        anchors.bottomMargin: root.theme.spacingSmall
                                        spacing: root.theme.spacingSmall

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: root.theme.spacingSmall

                                            Label {
                                                text: txRow.modelData && txRow.modelData.name ? txRow.modelData.name : ""
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                            Label {
                                                visible: root.sourceNames(txRow.modelData).length > 0
                                                text: qsTr("From: %1").arg(root.sourceNames(txRow.modelData))
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                                color: root.theme.textMuted
                                            }
                                        }

                                        Label {
                                            text: txRow.modelData && txRow.modelData.bookingDate ? txRow.modelData.bookingDate : ""
                                            Layout.preferredWidth: 110
                                            horizontalAlignment: Text.AlignRight
                                            elide: Text.ElideRight
                                        }

                                        Label {
                                            text: root.amountText(txRow.modelData && txRow.modelData.amount !== undefined ? txRow.modelData.amount : 0)
                                            Layout.preferredWidth: 90
                                            horizontalAlignment: Text.AlignRight
                                        }

                                        Label {
                                            text: txRow.modelData && txRow.modelData.allocatable ? qsTr("Allocatable") : qsTr("Non-allocatable")
                                            Layout.preferredWidth: 110
                                            horizontalAlignment: Text.AlignRight
                                            color: txRow.modelData && txRow.modelData.allocatable ? root.theme.success : root.theme.danger
                                        }

                                        Label {
                                            text: txRow.modelData && txRow.modelData.contractType && String(txRow.modelData.contractType).length > 0
                                                  ? String(txRow.modelData.contractType)
                                                  : qsTr("No type assigned")
                                            Layout.preferredWidth: 100
                                            horizontalAlignment: Text.AlignRight
                                            elide: Text.ElideRight
                                            color: txRow.modelData && txRow.modelData.contractType && String(txRow.modelData.contractType).length > 0
                                                   ? root.theme.textPrimary
                                                   : root.theme.textMuted
                                        }

                                        Label {
                                            text: txRow.modelData && txRow.modelData.statusText ? txRow.modelData.statusText : qsTr("Neutral")
                                            Layout.preferredWidth: 90
                                            horizontalAlignment: Text.AlignRight
                                            color: root.statusColor(txRow.modelData ? txRow.modelData.status : 0)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        visible: root.groupedCount() === 0
                        text: qsTr("No annual transactions")
                        color: root.theme.textMuted
                        width: txRows.width
                        leftPadding: root.theme.spacingSmall
                        rightPadding: root.theme.spacingSmall
                        topPadding: root.theme.spacingSmall
                        bottomPadding: root.theme.spacingSmall
                    }
                }
            }
        }
    }
}
