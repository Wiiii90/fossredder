/**
 * @file ui/tests/qml/analysis/tst_AnalysisTransactionsPanel.qml
 * @brief Provides QML tests for AnalysisTransactionsPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisTransactionsPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int formLabelWidth: 120
        property int formFieldWidth: 220
        property int spacingSmall: 6
        property int spacing: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textMuted: "#666666"
    }

    property var selectedIds: []
    property string calcName: ""
    property string calcPercent: ""
    property int applyCalls: 0

    Component {
        id: panelComponent
        AnalysisTransactionsPanel {
            width: 960
            height: 640
            theme: testCase.theme
            transactions: [
                { id: "tx-1", statementName: "S1", transactionName: "Rent", date: "2026-01-01", valuta: "2026-01-02", actorName: "Alice", contractName: "Lease", contractType: "lease", propertiesLabel: "Lot", amount: 100.0 }
            ]
            metrics: ({ statementCount: 1, transactionCount: 1, amountSum: 100.0 })
            selectedTransactionIds: testCase.selectedIds
            adjustedAmountsById: ({})
            calcName: testCase.calcName
            calcPercentText: testCase.calcPercent
            onSelectionChanged: (ids) => testCase.selectedIds = ids
            onCalcNameEdited: (name) => testCase.calcName = name
            onCalcPercentEdited: (percent) => testCase.calcPercent = percent
            onApplyCalcRequested: testCase.applyCalls += 1
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function init() {
        selectedIds = []
        calcName = ""
        calcPercent = ""
        applyCalls = 0
    }

    function test_selectionAndCalcSignals() {
        var panel = createPanel()
        var txCheck = findRequired(panel, "analysisTransactionSelectionCheckBox")
        var calcNameField = findRequired(panel, "analysisCalcNameField")
        var calcPercentField = findRequired(panel, "analysisCalcPercentField")
        var applyCalcButton = findRequired(panel, "analysisApplyCalcButton")

        txCheck.checked = true
        txCheck.clicked()
        compare(selectedIds.length, 1)
        compare(selectedIds[0], "tx-1")

        calcNameField.text = "VAT"
        calcPercentField.text = "19"
        compare(calcName, "VAT")
        compare(calcPercent, "19")

        applyCalcButton.clicked()
        compare(applyCalls, 1)
    }
}
