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
    readonly property string previewPngDataUrl: "data:image/svg+xml,%3Csvg width='1' height='1' viewBox='0 0 1 1'%3E%3Crect width='1' height='1' fill='red'/%3E%3C/svg%3E"

    property var session: QtObject {
        property string selectedAnalysisId: ""
        property int dataRevision: 0
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
        property int computeCalls: 0
        property string lastCreatedType: "plot"
        function contractTypes() { return ["lease"] }
        function analysisConfigJson(type, plotType, plotMeasure, propertyIds, contractTypes, taxPercent) {
            return JSON.stringify({ plotType: plotType, plotMeasure: plotMeasure, properties: propertyIds, contractTypes: contractTypes })
        }
        function analysisFilterSpec(dateField, dateMode, year, dateFrom, dateTo, propertyIds, contractTypes, allocatableMode) {
            var prefix = dateField === "valuta" ? "dateField=valuta;" : ""
            return prefix + (dateMode === "year"
                ? "date>=" + year + "-01-01;date<=" + year + "-12-31"
                : "date>=" + dateFrom + ";date<=" + dateTo)
        }
        function parseAnalysisFilterSpec(filterSpec) {
            var state = {
                dateField: "bookingDate",
                dateMode: "year",
                year: "2025",
                dateFrom: "",
                dateTo: "",
                propertyIds: [],
                propertyIdsNone: false,
                contractTypes: [],
                contractTypesNone: false,
                allocatableMode: "all"
            }
            var spec = String(filterSpec || "")
            if (spec.indexOf("dateField=valuta") !== -1)
                state.dateField = "valuta"
            return state
        }
        function analysisAdjustmentsJson(transactions, selectedTransactionIds, taxPercent) { return "{}" }
        function previewTransactions(filterSpec) { return { transactions: [], metrics: { statementCount: 0, transactionCount: 0, amountSum: 0.0 } } }
        function computeAnalysis(analysisId, filterSpec) {
            computeCalls += 1
            if (lastCreatedType === "tab") {
                return ({
                    type: "tab",
                    table: [
                        ["2026-01-01", "Rent", "100.0"],
                        ["2026-01-02", "Service", "50.0"]
                    ]
                })
            }
            return ({
                type: "pie",
                artifacts: [testCase.previewPngDataUrl],
                table: [["Lease", "100.0"]]
            })
        }
        function createAnalysis(name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) {
            lastCreatedType = type
            session.analysesData = [{
                id: "analysis-new",
                name: name,
                type: type,
                config: configJson,
                filter: filterSpec,
                exportFormat: exportFormat,
                includeCalcAdjustments: includeCalcAdjustments,
                exportState: exportStateJson,
                snapshotTransactions: snapshotTransactionsJson,
                adjustments: "{}"
            }]
            session.dataRevision += 1
            return "analysis-new"
        }
        function updateAnalysis(id, name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) {}
        function deleteAnalysis(id) {}
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var analysisWorkflow: testCase.analysisController
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
        var view = createTemporaryObject(analysisViewComponent, testCase)
        view.visible = true
        return view
    }

    function init() {
        session.selectedAnalysisId = ""
        session.analysesData = []
        session.propertiesData = [{ id: "property-1", name: "Lot" }]
        session.lastAnalysisResult = ({})
        analysisController.computeCalls = 0
        analysisController.lastCreatedType = "plot"
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

    function test_createAnalysisShowsRenderedPreviewImage() {
        var view = createView()
        var nameField = findRequired(view, "analysisNameField")
        var createButton = findRequired(view, "analysisCreateButton")
        var beforeComputeCalls = analysisController.computeCalls

        nameField.text = "Preview"
        createButton.clicked()

        verify(analysisController.computeCalls > beforeComputeCalls)
        compare(session.selectedAnalysisId, "analysis-new")
        wait(0)

        var image = findRequired(view, "analysisPreviewImage")
        compare(String(image.source), previewPngDataUrl)
        tryCompare(image, "status", Image.Ready)
    }

    function test_createTableAnalysisShowsTablePreview() {
        var view = createView()
        var nameField = findRequired(view, "analysisNameField")
        var typeCombo = findRequired(view, "analysisMainTypeComboBox")
        var createButton = findRequired(view, "analysisCreateButton")
        var beforeComputeCalls = analysisController.computeCalls

        nameField.text = "Table Preview"
        typeCombo.currentIndex = 1
        createButton.clicked()

        verify(analysisController.computeCalls > beforeComputeCalls)
        compare(session.selectedAnalysisId, "analysis-new")
        wait(0)

        var table = findRequired(view, "analysisTablePreview")
        var viewport = findRequired(view, "analysisTableViewport")
        verify(table.width > 0)
        verify(table.height > 0)
        verify(viewport.width > 0)
        verify(viewport.height > 0)
        tryCompare(table, "grandTotal", 150.0)
        compare(table.contractTypes.length, 1)
        compare(table.matrixPropertyNames.length, 1)
    }

    function test_navigationCyclesThroughCreateMode() {
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" },
            { id: "analysis-2", name: "A2", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" },
            { id: "analysis-3", name: "A3", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" }
        ]
        session.selectedAnalysisId = "analysis-3"

        var view = createView()
        findRequired(view, "analysisNextButton").clicked()
        compare(session.selectedAnalysisId, "")

        findRequired(view, "analysisNextButton").clicked()
        compare(session.selectedAnalysisId, "analysis-1")

        session.selectedAnalysisId = "analysis-1"
        findRequired(view, "analysisPreviousButton").clicked()
        compare(session.selectedAnalysisId, "")

        findRequired(view, "analysisPreviousButton").clicked()
        compare(session.selectedAnalysisId, "analysis-3")
    }

    function test_navigationStaysEnabledWithSingleRow() {
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" }
        ]

        var view = createView()
        var nextButton = findRequired(view, "analysisNextButton")
        var previousButton = findRequired(view, "analysisPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedAnalysisId, "analysis-1")
    }

    function test_createModeNavigationStartsAtEdges() {
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" },
            { id: "analysis-2", name: "A2", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" },
            { id: "analysis-3", name: "A3", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" }
        ]

        var view = createView()
        findRequired(view, "analysisNextButton").clicked()
        compare(session.selectedAnalysisId, "analysis-1")

        session.selectedAnalysisId = ""
        findRequired(view, "analysisPreviousButton").clicked()
        compare(session.selectedAnalysisId, "analysis-3")
    }
}
