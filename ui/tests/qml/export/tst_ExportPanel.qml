/**
 * @file ui/tests/qml/export/tst_ExportPanel.qml
 * @brief Provides QML tests for ExportPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var annualsData: []
        property var analysesData: []
        function annualRows() { return annualsData || [] }
        function analysisRows() { return analysesData || [] }
    }

    property var appContext: QtObject {
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacing: 8
        property int spacingSmall: 6
        property int formFieldWidth: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property int viewInlineIconSize: 28
        property int viewSectionIconSize: 42
        property int margins: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color subtlePrimaryFill: "#eef3ff"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: exportPanelComponent
        ExportPanel {
            width: 920
            height: 560
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(exportPanelComponent, testCase)
    }

    function init() {
        session.annualsData = [
            { id: "annual-1", name: "Annual 1", display: "Annual 1", assignedAnalysisIds: ["analysis-1"] }
        ]
        session.analysesData = [
            { id: "analysis-1", name: "Analysis 1", type: "tab", exportFormat: "CSV" },
            { id: "analysis-2", name: "Analysis 2", type: "plot", exportFormat: "PNG" }
        ]
    }

    function test_EXP_P_001_and_EXP_P_008_addAnnualCreatesAnnualAndAnalysisExportItems() {
        var view = createView()
        var addButton = findRequired(view, "exportAddEntryButton")

        addButton.clicked()
        var items = view.exportItems()

        compare(items.length, 2)
        compare(items[0].objectType, "Annual")
        compare(items[0].objectId, "annual-1")
        compare(items[1].objectType, "Analysis")
        compare(items[1].annualId, "annual-1")
        compare(items[1].objectId, "analysis-1")
    }

    function test_EXP_P_002_addStandaloneAnalysisCreatesAnalysisItem() {
        var view = createView()
        var analysisModeButton = findRequired(view, "exportAddAnalysisModeButton")
        var addButton = findRequired(view, "exportAddEntryButton")

        analysisModeButton.clicked()
        addButton.clicked()
        var items = view.exportItems()

        compare(items.length, 1)
        compare(items[0].objectType, "Analysis")
        compare(items[0].annualId, "")
    }

    function test_EXP_P_003_clearAllRemovesEntries() {
        var view = createView()
        var addButton = findRequired(view, "exportAddEntryButton")

        addButton.clicked()
        verify(view.exportItems().length > 0)

        view.clearAll()
        compare(view.exportItems().length, 0)
    }

    function test_EXP_P_007_loadItemsReconstructsEntriesDeterministically() {
        var view = createView()

        view.loadItems([
            { objectType: "Annual", objectId: "annual-1", objectName: "Annual 1" },
            { objectType: "Analysis", annualId: "annual-1", objectId: "analysis-1", objectName: "Analysis 1", exportType: "CSV" },
            { objectType: "Analysis", annualId: "", objectId: "analysis-2", objectName: "Analysis 2", exportType: "PNG" }
        ])

        var items = view.exportItems()
        verify(items.length >= 3)
        compare(items[0].objectType, "Annual")
        compare(items[0].objectId, "annual-1")
    }

}
