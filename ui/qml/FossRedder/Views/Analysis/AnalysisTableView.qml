/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Analysis/AnalysisTableView.qml
 * @brief Provides the AnalysisTableView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

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

    function propertyLabel(propertyId) {
        try {
            if (root.session && root.session.propertyName)
                return root.session.propertyName(propertyId)
        } catch (e) {
        }
        return propertyId
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
            return root.theme.surfaceAlt
        return alternateRow ? Qt.lighter(root.theme.surfaceAlt, 1.02) : root.theme.surface
    }

    function gridBorderColor(isHeader, isTotal) {
        return (isHeader || isTotal) ? root.theme.borderStrong : root.theme.borderLight
    }

    function rebuildMatrix() {
        const result = root.session ? root.session.lastAnalysisResult : null
        const transactions = result && result.transactions ? result.transactions : []

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
                    : []

            contractSet[contractType] = true
            if (!amountsByContract[contractType])
                amountsByContract[contractType] = ({})
            contractTotals[contractType] = (contractTotals[contractType] || 0.0) + adjustedAmount
            grandTotal += adjustedAmount

            for (let p = 0; p < propertyIds.length; ++p) {
                const propertyId = String(propertyIds[p])
                if (propertyId.length === 0)
                    continue
                const propertyName = root.propertyLabel(propertyId)
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

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Flickable {
            id: matrixViewport
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
                                root.theme.analysis.table.propertyColumnWidth
                                + Math.max(1, root.contractTypes.length) * root.theme.analysis.table.amountColumnWidth
                                + root.theme.analysis.table.totalColumnWidth)
                spacing: 0

                Rectangle {
                    visible: root.matrixPropertyNames.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        Label {
                            text: qsTr("Property")
                            Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                            leftPadding: root.theme.spacingSmall
                            elide: Text.ElideRight
                        }
                        Repeater {
                            model: root.contractTypes
                            Label {
                                required property var modelData
                                text: modelData
                                Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                horizontalAlignment: Text.AlignRight
                                rightPadding: root.theme.spacingSmall
                                elide: Text.ElideRight
                            }
                        }
                        Label {
                            text: qsTr("Total")
                            Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                            horizontalAlignment: Text.AlignRight
                            rightPadding: root.theme.spacingSmall
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
                        Layout.preferredHeight: root.theme.analysis.table.rowHeight
                        color: index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                        border.width: root.theme.borderWidthThin
                        border.color: root.theme.borderLight

                        RowLayout {
                            anchors.fill: parent
                            spacing: 0

                            Label {
                                text: rowContainer.modelData
                                Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                                leftPadding: root.theme.spacingSmall
                                elide: Text.ElideRight
                            }

                            Repeater {
                                model: root.contractTypes

                                Rectangle {
                                    id: valueCell
                                    required property var modelData
                                    Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                    color: rowContainer.index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderLight

                                    Label {
                                        anchors.fill: parent
                                        text: root.matrixValue(valueCell.modelData, rowContainer.modelData).toFixed(2)
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: root.theme.spacingSmall
                                    }
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                                Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                color: root.theme.surfaceAlt
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.borderStrong

                                Label {
                                    anchors.fill: parent
                                    text: root.propertyTotal(rowContainer.modelData).toFixed(2)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    rightPadding: root.theme.spacingSmall
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    visible: root.matrixPropertyNames.length > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderStrong

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        Label {
                            text: qsTr("Total")
                            Layout.preferredWidth: root.theme.analysis.table.propertyColumnWidth
                            leftPadding: root.theme.spacingSmall
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                            color: root.theme.textPrimary
                        }

                        Repeater {
                            model: root.contractTypes

                            Rectangle {
                                id: contractTotalCell
                                required property var modelData
                                Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                Layout.preferredHeight: root.theme.analysis.table.rowHeight
                                color: root.gridCellColor(false, true, false)
                                border.width: root.theme.borderWidthThin
                                border.color: root.gridBorderColor(false, true)

                                Label {
                                    anchors.fill: parent
                                    text: root.contractTotal(contractTotalCell.modelData).toFixed(2)
                                    rightPadding: root.theme.spacingSmall
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: root.theme.analysis.table.totalColumnWidth
                            Layout.preferredHeight: root.theme.analysis.table.rowHeight
                            color: root.gridCellColor(false, true, false)
                            border.width: root.theme.borderWidthThin
                            border.color: root.gridBorderColor(false, true)

                            Label {
                                anchors.fill: parent
                                text: root.grandTotal.toFixed(2)
                                rightPadding: root.theme.spacingSmall
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
                    Layout.preferredHeight: root.theme.analysis.table.rowHeight * 2
                    color: root.theme.surfaceAlt
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderLight

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("No table preview available")
                        color: root.theme.textMuted
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
