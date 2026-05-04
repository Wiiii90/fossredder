/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisTransactionsPanel.qml
 * @brief Provides the AnalysisTransactionsPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var transactions
    required property var metrics
    property var selectedTransactionIds: []
    property var adjustedAmountsById: ({})
    property string calcName: ""
    property string calcPercentText: ""

    signal selectionChanged(var ids)
    signal calcNameEdited(string name)
    signal calcPercentEdited(string percent)
    signal applyCalcRequested()

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Calc name")
                Layout.preferredWidth: root.theme.formLabelWidth
            }
            Controls.TextField {
                id: calcNameField
                objectName: "analysisCalcNameField"
                Layout.preferredWidth: root.theme.formFieldWidth
                placeholderText: qsTr("e.g. VAT")
                text: root.calcName
                onTextChanged: root.calcNameEdited(text)
            }
            Label { text: qsTr("Calc %") }
            Controls.TextField {
                id: calcPercentField
                objectName: "analysisCalcPercentField"
                Layout.preferredWidth: 90
                placeholderText: qsTr("0")
                text: root.calcPercentText
                onTextChanged: root.calcPercentEdited(text)
            }
            Controls.Button {
                objectName: "analysisApplyCalcButton"
                text: qsTr("Apply to selected")
                onClicked: root.applyCalcRequested()
            }
        }

        Flickable {
            id: txViewport
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: txTable.width
            contentHeight: txTable.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            Item {
                id: txTable
                width: Math.max(txViewport.width,
                                44 + 130 + 150 + 110 + 110 + 120 + 120 + 90 + 220 + 160
                                + root.theme.spacingSmall * 9
                                + root.theme.spacingSmall * 2)
                implicitHeight: headerRect.height + txList.contentHeight

                Rectangle {
                    id: headerRect
                    width: txTable.width
                    height: 32
                    color: root.theme.surfaceAlt
                    border.width: 1
                    border.color: root.theme.border

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: root.theme.spacingSmall
                        anchors.rightMargin: root.theme.spacingSmall
                        spacing: root.theme.spacingSmall
                        Label { text: qsTr("Apply"); Layout.preferredWidth: 44 }
                        Label { text: qsTr("Statement"); Layout.preferredWidth: 130 }
                        Label { text: qsTr("Transaction"); Layout.preferredWidth: 150 }
                        Label { text: qsTr("Booking Date"); Layout.preferredWidth: 110 }
                        Label { text: qsTr("Valuta"); Layout.preferredWidth: 110 }
                        Label { text: qsTr("Actor"); Layout.preferredWidth: 120 }
                        Label { text: qsTr("Contract"); Layout.preferredWidth: 120 }
                        Label { text: qsTr("Type"); Layout.preferredWidth: 90 }
                        Label { text: qsTr("Properties"); Layout.preferredWidth: 220; elide: Text.ElideRight }
                        Label { text: qsTr("Amount"); Layout.preferredWidth: 160; horizontalAlignment: Text.AlignRight }
                    }
                }

                ListView {
                    id: txList
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: headerRect.bottom
                    height: Math.max(0, txViewport.height - headerRect.height)
                    clip: true
                    model: root.transactions
                    delegate: Rectangle {
                        id: txRow
                        required property var modelData
                        required property int index
                        width: txTable.width
                        height: 30
                        color: index % 2 === 0 ? root.theme.surface : root.theme.surfaceAlt
                        property string txId: modelData && modelData.id ? modelData.id : ""
                        property double amountValue: modelData && modelData.amount !== undefined ? Number(modelData.amount) : 0.0
                        property bool hasAdjusted: root.adjustedAmountsById && root.adjustedAmountsById[txId] !== undefined
                        property double adjustedValue: hasAdjusted ? Number(root.adjustedAmountsById[txId]) : amountValue

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.rightMargin: root.theme.spacingSmall
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "analysisTransactionSelectionCheckBox"
                                Layout.preferredWidth: 44
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.selectedTransactionIds.indexOf(txRow.txId) !== -1
                                onClicked: {
                                    const next = root.selectedTransactionIds ? root.selectedTransactionIds.slice() : []
                                    const idx = next.indexOf(txRow.txId)
                                    if (checked && idx === -1)
                                        next.push(txRow.txId)
                                    if (!checked && idx !== -1)
                                        next.splice(idx, 1)
                                    root.selectionChanged(next)
                                }
                            }

                            Label { text: txRow.modelData && txRow.modelData.statementName ? txRow.modelData.statementName : ""; Layout.preferredWidth: 130; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.transactionName ? txRow.modelData.transactionName : ""; Layout.preferredWidth: 150; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.date ? txRow.modelData.date : ""; Layout.preferredWidth: 110; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.valuta ? txRow.modelData.valuta : ""; Layout.preferredWidth: 110; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.actorName ? txRow.modelData.actorName : ""; Layout.preferredWidth: 120; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.contractName ? txRow.modelData.contractName : ""; Layout.preferredWidth: 120; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.contractType ? txRow.modelData.contractType : qsTr("Unassigned"); Layout.preferredWidth: 90; elide: Text.ElideRight }
                            Label { text: txRow.modelData && txRow.modelData.propertiesLabel ? txRow.modelData.propertiesLabel : ""; Layout.preferredWidth: 220; elide: Text.ElideRight }
                            Label {
                                Layout.preferredWidth: 160
                                horizontalAlignment: Text.AlignRight
                                text: txRow.hasAdjusted
                                      ? (txRow.amountValue.toFixed(2) + " (" + txRow.adjustedValue.toFixed(2) + " " + qsTr("plus") + " " + (root.calcName.length > 0 ? root.calcName : qsTr("calc")) + ")")
                                      : txRow.amountValue.toFixed(2)
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: root.theme.border
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Statements: %1").arg(root.metrics && root.metrics.statementCount !== undefined ? root.metrics.statementCount : 0); Layout.preferredWidth: 160 }
            Label { text: qsTr("Transactions: %1").arg(root.metrics && root.metrics.transactionCount !== undefined ? root.metrics.transactionCount : 0); Layout.preferredWidth: 170 }
            Item { Layout.fillWidth: true }
            Label { text: qsTr("Amount sum: %1").arg(Number(root.metrics && root.metrics.amountSum !== undefined ? root.metrics.amountSum : 0).toFixed(2)); Layout.preferredWidth: 180; horizontalAlignment: Text.AlignRight }
        }
    }
}
