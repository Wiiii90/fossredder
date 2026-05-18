/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisTableView.qml
 * @brief Provides the AnalysisTableView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    objectName: "analysisTablePreview"
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var analysisTableTheme: root.theme && root.theme.analysis && root.theme.analysis.table ? root.theme.analysis.table : ({})
    readonly property int tablePropertyColumnWidthValue: root.theme && root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.propertyColumnWidth === "number" ? root.theme.analysis.table.propertyColumnWidth : 180
    readonly property int tableAmountColumnWidthValue: root.theme && root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.amountColumnWidth === "number" ? root.theme.analysis.table.amountColumnWidth : 120
    readonly property int tableTotalColumnWidthValue: root.theme && root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.totalColumnWidth === "number" ? root.theme.analysis.table.totalColumnWidth : 140
    readonly property int tableRowHeightValue: root.theme && root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.rowHeight === "number" ? root.theme.analysis.table.rowHeight : 30
    readonly property int spacingSmallValue: root.theme && typeof root.theme.spacingSmall === "number" ? root.theme.spacingSmall : 6
    readonly property int borderWidthThinValue: root.theme && typeof root.theme.borderWidthThin === "number" ? root.theme.borderWidthThin : 1
    readonly property color surfaceColor: root.theme && root.theme.surface ? root.theme.surface : "#ffffff"
    readonly property color surfaceAltColor: root.theme && root.theme.surfaceAlt ? root.theme.surfaceAlt : "#f5f5f5"
    readonly property color textPrimaryColor: root.theme && root.theme.textPrimary ? root.theme.textPrimary : "#202020"
    readonly property color textMutedColor: root.theme && root.theme.textMuted ? root.theme.textMuted : "#666666"
    readonly property color borderLightColor: root.theme && root.theme.borderLight ? root.theme.borderLight : "#d7d7d7"
    readonly property color borderStrongColor: root.theme && root.theme.borderStrong ? root.theme.borderStrong : "#888888"

    property var adjustmentAmountsById: ({})
    property var contractTypes: []
    property var matrixRows: []
    property var matrixPropertyNames: []
    property var contractTotals: ({})
    property var propertyTotals: ({})
    property real grandTotal: 0.0

    function parseNumber(value) {
        const n = parseFloat(value)
        return isNaN(n) ? 0.0 : n
    }

    function propertyLabel(tx, index, propertyId) {
        const names = tx && tx.propertyNames && tx.propertyNames.length !== undefined ? tx.propertyNames : []
        if (index >= 0 && index < names.length) {
            const name = String(names[index] || "")
            if (name.length > 0)
                return name
        }
        return String(propertyId || "")
    }

    function matrixValue(contractType, propertyName) {
        if (!root.contractTotals || !root.matrixRows)
            return 0.0
        for (let i = 0; i < root.matrixRows.length; ++i) {
            const row = root.matrixRows[i]
            if (row && row.contractType === contractType && row.amountsByProperty) {
                const value = row.amountsByProperty[propertyName]
                return value === undefined ? 0.0 : parseNumber(value)
            }
        }
        return 0.0
    }

    function contractTotal(contractType) {
        const value = root.contractTotals ? root.contractTotals[contractType] : undefined
        return value === undefined ? 0.0 : parseNumber(value)
    }

    function propertyTotal(propertyName) {
        const value = root.propertyTotals ? root.propertyTotals[propertyName] : undefined
        return value === undefined ? 0.0 : parseNumber(value)
    }

    function gridCellColor(isHeader, isTotal, alternateRow) {
        if (isHeader || isTotal)
            return root.surfaceAltColor
        return alternateRow ? Qt.lighter(root.surfaceAltColor, 1.02) : root.surfaceColor
    }

    function gridBorderColor(isHeader, isTotal) {
        return (isHeader || isTotal) ? root.borderStrongColor : root.borderLightColor
    }

    function rebuildMatrix() {
        const result = root.session ? root.session.lastAnalysisResult : null
        const transactions = root.tableTransactions(result)

        root.matrixPropertyNames = []
        root.contractTotals = ({})
        root.propertyTotals = ({})
        root.grandTotal = 0.0

        const amountsByContract = ({})
        const propertySet = ({})
        const contractSet = ({})
        const propertyTotals = ({})
        const contractTotals = ({})
        let grandTotal = 0.0

        for (let t = 0; t < transactions.length; ++t) {
            const tx = transactions[t]
            if (!tx)
                continue

            const contractType = tx.contractType && String(tx.contractType).length > 0
                    ? String(tx.contractType)
                    : qsTr("Unassigned")
            const txId = tx.id ? String(tx.id) : ""
            const baseAmount = parseNumber(tx.amount)
            const adjustedAmount = txId.length > 0 && root.adjustmentAmountsById && root.adjustmentAmountsById[txId] !== undefined
                    ? parseNumber(root.adjustmentAmountsById[txId])
                    : baseAmount
            const propertyIds = tx.propertyIds && tx.propertyIds.length > 0
                    ? tx.propertyIds
                    : [qsTr("Unassigned")]

            contractSet[contractType] = true
            if (!amountsByContract[contractType])
                amountsByContract[contractType] = ({})
            contractTotals[contractType] = (contractTotals[contractType] || 0.0) + adjustedAmount
            grandTotal += adjustedAmount

            for (let p = 0; p < propertyIds.length; ++p) {
                const propertyId = String(propertyIds[p])
                if (propertyId.length === 0)
                    continue
                const propertyName = root.propertyLabel(tx, p, propertyId)
                propertySet[propertyName] = true
                amountsByContract[contractType][propertyName] = (amountsByContract[contractType][propertyName] || 0.0) + adjustedAmount
                propertyTotals[propertyName] = (propertyTotals[propertyName] || 0.0) + adjustedAmount
            }
        }

        const txContracts = Object.keys(contractSet).sort()
        const txProperties = Object.keys(propertySet).sort()
        if (txContracts.length > 0 && txProperties.length > 0) {
            const txRows = []
            for (let c = 0; c < txContracts.length; ++c) {
                const contractType = txContracts[c]
                txRows.push({
                    contractType: contractType,
                    amountsByProperty: amountsByContract[contractType] ? amountsByContract[contractType] : ({})
                })
            }

            root.contractTypes = txContracts
            root.matrixPropertyNames = txProperties
            root.matrixRows = txRows
            root.contractTotals = contractTotals
            root.propertyTotals = propertyTotals
            root.grandTotal = grandTotal
            return
        }

        root.contractTypes = []
        root.matrixRows = []
    }

    function tableTransactions(result) {
        if (!result)
            return []
        if (result.transactions && result.transactions.length !== undefined && result.transactions.length > 0)
            return result.transactions

        const table = result.table && result.table.length !== undefined ? result.table : []
        const out = []
        for (let i = 0; i < table.length; ++i) {
            const row = table[i]
            if (!row || row.length < 3)
                continue
            out.push({
                id: "table-row-" + i,
                date: String(row[0] || ""),
                name: String(row[1] || ""),
                amount: root.parseNumber(row[2]),
                contractType: qsTr("Unassigned"),
                propertyIds: [qsTr("Unassigned")]
            })
        }
        return out
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.spacingSmallValue

        Flickable {
            id: matrixViewport
            objectName: "analysisTableViewport"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: matrixContent.width
            contentHeight: matrixContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            ColumnLayout {
                id: matrixContent
                width: Math.max(matrixViewport.width,
                                root.tablePropertyColumnWidthValue
                                + Math.max(1, root.contractTypes.length) * root.tableAmountColumnWidthValue
                                + root.tableTotalColumnWidthValue)
                spacing: 0

                Rectangle {
                    visible: root.matrixPropertyNames.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.tableRowHeightValue
                    color: root.surfaceAltColor
                    border.width: root.borderWidthThinValue
                    border.color: root.borderStrongColor

                    RowLayout {
                        width: parent ? parent.width : 0
                        height: parent ? parent.height : implicitHeight
                        spacing: 0
                        Label {
                            text: qsTr("Property")
                            Layout.preferredWidth: root.tablePropertyColumnWidthValue
                            leftPadding: root.spacingSmallValue
                            elide: Text.ElideRight
                        }
                        Repeater {
                            model: root.contractTypes
                            Label {
                                required property var modelData
                                text: modelData
                                Layout.preferredWidth: root.tableAmountColumnWidthValue
                                horizontalAlignment: Text.AlignRight
                                rightPadding: root.spacingSmallValue
                                elide: Text.ElideRight
                            }
                        }
                        Label {
                            text: qsTr("Total")
                            Layout.preferredWidth: root.tableTotalColumnWidthValue
                            Layout.minimumWidth: root.tableTotalColumnWidthValue
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            rightPadding: root.spacingSmallValue
                        }
                    }
                }

                Repeater {
                    visible: root.matrixPropertyNames.length > 0
                    model: root.matrixPropertyNames
                    Rectangle {
                        id: rowContainer
                        required property int index
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.tableRowHeightValue
                        color: index % 2 === 0 ? root.surfaceColor : root.surfaceAltColor
                        border.width: root.borderWidthThinValue
                        border.color: root.borderLightColor

                        RowLayout {
                            anchors.fill: parent
                            spacing: 0

                            Label {
                                text: rowContainer.modelData
                                Layout.preferredWidth: root.tablePropertyColumnWidthValue
                                leftPadding: root.spacingSmallValue
                                elide: Text.ElideRight
                            }

                            Repeater {
                                model: root.contractTypes

                                Rectangle {
                                    id: valueCell
                                    required property var modelData
                                    Layout.preferredWidth: root.tableAmountColumnWidthValue
                                    Layout.preferredHeight: root.tableRowHeightValue
                                    color: rowContainer.index % 2 === 0 ? root.surfaceColor : root.surfaceAltColor
                                    border.width: root.borderWidthThinValue
                                    border.color: root.borderLightColor

                                    Label {
                                        anchors.fill: parent
                                        text: root.matrixValue(valueCell.modelData, rowContainer.modelData).toFixed(2)
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: root.spacingSmallValue
                                    }
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: root.tableTotalColumnWidthValue
                                Layout.minimumWidth: root.tableTotalColumnWidthValue
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.tableRowHeightValue
                                color: root.surfaceAltColor
                                border.width: root.borderWidthThinValue
                                border.color: root.borderStrongColor

                                Label {
                                    anchors.fill: parent
                                    text: root.propertyTotal(rowContainer.modelData).toFixed(2)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    rightPadding: root.spacingSmallValue
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.matrixPropertyNames.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.tableRowHeightValue
                    color: root.surfaceAltColor
                    border.width: root.borderWidthThinValue
                    border.color: root.borderStrongColor

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        Label {
                            text: qsTr("Total")
                            Layout.preferredWidth: root.tablePropertyColumnWidthValue
                            leftPadding: root.spacingSmallValue
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                            color: root.textPrimaryColor
                        }

                        Repeater {
                            model: root.contractTypes

                            Rectangle {
                                id: contractTotalCell
                                required property var modelData
                                Layout.preferredWidth: root.tableAmountColumnWidthValue
                                Layout.preferredHeight: root.tableRowHeightValue
                                color: root.gridCellColor(false, true, false)
                                border.width: root.borderWidthThinValue
                                border.color: root.gridBorderColor(false, true)

                                Label {
                                    anchors.fill: parent
                                    text: root.contractTotal(contractTotalCell.modelData).toFixed(2)
                                    rightPadding: root.spacingSmallValue
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: root.tableTotalColumnWidthValue
                            Layout.minimumWidth: root.tableTotalColumnWidthValue
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.tableRowHeightValue
                            color: root.gridCellColor(false, true, false)
                            border.width: root.borderWidthThinValue
                            border.color: root.gridBorderColor(false, true)

                            Label {
                                anchors.fill: parent
                                text: root.grandTotal.toFixed(2)
                                rightPadding: root.spacingSmallValue
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.matrixPropertyNames.length === 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.tableRowHeightValue * 2
                    color: root.surfaceAltColor
                    border.width: root.borderWidthThinValue
                    border.color: root.borderLightColor

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("No table preview available")
                        color: root.textMutedColor
                    }
                }
            }
        }
    }

    Connections {
        target: root.session
        function onLastAnalysisResultChanged() { root.rebuildMatrix() }
    }

    Component.onCompleted: root.rebuildMatrix()
}
