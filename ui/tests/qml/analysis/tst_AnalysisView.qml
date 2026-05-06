/**
 * @file ui/tests/qml/analysis/tst_AnalysisView.qml
 * @brief Provides QML tests for AnalysisView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property string selectedAnalysisId: ""
        property var analysesData: []
        property var propertiesData: []
        property var lastAnalysisResult: ({})
        property var analyses: QtObject {
            function setAdjustmentsById(id, adjustments) {}
        }
        function analysisRows() { return analysesData || [] }
        function propertyRows() { return propertiesData || [] }
    }

    property var analysisController: QtObject {
        function contractTypes() { return ["lease"] }
        function analysisConfigJson(type, plotType, plotMeasure, propertyIds, contractTypes, taxPercent) { return "{}" }
        function analysisFilterSpec(dateMode, year, dateFrom, dateTo, propertyIds, contractTypes, allocatableMode) { return "" }
        function analysisAdjustmentsJson(transactions, selectedTransactionIds, taxPercent) { return "{}" }
        function previewTransactions(filterSpec) { return { transactions: [], metrics: { statementCount: 0, transactionCount: 0, amountSum: 0.0 } } }
        function computeAnalysis(analysisId, filterSpec) { return ({}) }
        function createAnalysis(name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) { return "analysis-new" }
        function updateAnalysis(id, name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) {}
        function deleteAnalysis(id) {}
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var analysisController: testCase.analysisController
        property bool isDebugBuild: false
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacingSmall: 6
        property int spacing: 8
        property int spacingMedium: 10
        property int spacingLarge: 16
        property int panelPadding: 8
        property int panelContentSafeWidthOffset: 40
        property int controlHeight: 32
        property int formFieldWidth: 220
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int viewActionButtonWidth: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color borderLight: "#d7d7d7"
        property color borderStrong: "#888888"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color chartText: "#000000"
        property color chartFallback: "#77aadd"
        property var analysis: QtObject {
            property var palette: ["#77aadd", "#88ccaa", "#ddaa77", "#cc88aa"]
            property var layout: QtObject {
                property int splitControlsWidth: 180
            }
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
        }
        property int chartPlotMinimumHeight: 180
        property int chartPlotPreferredHeight: 260
        property int chartLegendHeight: 80
        property int chartLegendMarkerSize: 12
        property int chartValueLabelWidth: 70
        property int chartPercentLabelWidth: 58
        property string fontFamily: "Segoe UI"
        property int fontSize: 10
    }

    Component {
        id: analysisViewComponent
        AnalysisView {
            width: 960
            height: 640
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(analysisViewComponent, testCase)
    }

    function init() {
        session.selectedAnalysisId = ""
        session.analysesData = []
        session.propertiesData = [{ id: "property-1", name: "Lot" }]
        session.lastAnalysisResult = ({})
    }

    function test_mountsAnalysisForm() {
        var view = createView()
        var nameField = findRequired(view, "analysisNameField")
        verify(nameField !== null)
    }

    function test_visibleRefreshKeepsSelectionBound() {
        session.analysesData = [{ id: "analysis-1", name: "A", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" }]
        session.selectedAnalysisId = "analysis-1"

        var view = createView()
        view.visible = false
        view.visible = true

        var updateButton = findRequired(view, "analysisUpdateButton")
        verify(updateButton !== null)
    }
}
