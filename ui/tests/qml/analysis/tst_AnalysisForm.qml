/**
 * @file ui/tests/qml/analysis/tst_AnalysisForm.qml
 * @brief Provides QML tests for AnalysisForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisFormTests"
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
        property int createCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        property int previewCalls: 0
        property int computeCalls: 0
        property var lastCreate: ({})
        property var lastUpdate: ({})
        property string lastDeleteId: ""
        property var availableContractTypes: ["lease", "service"]

        function reset() {
            createCalls = 0
            updateCalls = 0
            deleteCalls = 0
            previewCalls = 0
            computeCalls = 0
            lastCreate = ({})
            lastUpdate = ({})
            lastDeleteId = ""
        }

        function contractTypes() {
            return availableContractTypes
        }

        function analysisConfigJson(type, plotType, plotMeasure, propertyIds, contractTypes, taxPercent) {
            return JSON.stringify({
                type: type,
                plotType: plotType,
                plotMeasure: plotMeasure,
                propertyIds: propertyIds,
                contractTypes: contractTypes,
                taxPercent: taxPercent
            })
        }

        function analysisFilterSpec(dateMode, year, dateFrom, dateTo, propertyIds, contractTypes, allocatableMode) {
            if (dateMode === "year")
                return "date>=" + year + "-01-01;date<=" + year + "-12-31;allocatable=" + allocatableMode
            return "date>=" + dateFrom + ";date<=" + dateTo + ";allocatable=" + allocatableMode
        }

        function analysisAdjustmentsJson(transactions, selectedTransactionIds, taxPercent) {
            return JSON.stringify({ selected: selectedTransactionIds, percent: taxPercent })
        }

        function previewTransactions(filterSpec) {
            previewCalls += 1
            return {
                transactions: [{ id: "tx-1", transactionName: "Rent", amount: 10.0, statementName: "S1" }],
                metrics: { statementCount: 1, transactionCount: 1, amountSum: 10.0 }
            }
        }

        function computeAnalysis(analysisId, filterSpec) {
            computeCalls += 1
            return { result: "ok", id: analysisId, filter: filterSpec }
        }

        function createAnalysis(name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) {
            createCalls += 1
            lastCreate = {
                name: name,
                type: type,
                configJson: configJson,
                filterSpec: filterSpec,
                exportFormat: exportFormat,
                includeCalcAdjustments: includeCalcAdjustments,
                exportStateJson: exportStateJson,
                snapshotTransactionsJson: snapshotTransactionsJson
            }
            return "analysis-new"
        }

        function updateAnalysis(id, name, type, configJson, filterSpec, exportFormat, includeCalcAdjustments, exportStateJson, snapshotTransactionsJson) {
            updateCalls += 1
            lastUpdate = {
                id: id,
                name: name,
                type: type,
                configJson: configJson,
                filterSpec: filterSpec,
                exportFormat: exportFormat,
                includeCalcAdjustments: includeCalcAdjustments,
                exportStateJson: exportStateJson,
                snapshotTransactionsJson: snapshotTransactionsJson
            }
        }

        function deleteAnalysis(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
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
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: analysisFormComponent
        AnalysisForm {
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

    function createForm() {
        return createTemporaryObject(analysisFormComponent, testCase)
    }

    function init() {
        analysisController.reset()
        session.selectedAnalysisId = ""
        session.analysesData = []
        session.propertiesData = [
            { id: "property-1", name: "Lot" },
            { id: "property-2", name: "House" }
        ]
        session.lastAnalysisResult = ({})
    }

    function test_createAnalysisUpdatesSelection() {
        var form = createForm()
        var nameField = findRequired(form, "analysisNameField")
        var createButton = findRequired(form, "analysisCreateButton")

        nameField.text = "AN 1"
        createButton.clicked()

        compare(analysisController.createCalls, 1)
        compare(analysisController.lastCreate.name, "AN 1")
        compare(session.selectedAnalysisId, "analysis-new")
    }

    function test_readModeLoadsSelectedAnalysisState() {
        session.analysesData = [{
            id: "analysis-1",
            name: "Loaded",
            type: "tab",
            config: "{}",
            filter: "date>=2026-01-01;date<=2026-12-31;allocatable=all",
            exportFormat: "invalid",
            includeCalcAdjustments: true,
            exportState: "{}",
            snapshotTransactions: "[]",
            adjustments: "{}"
        }]
        session.selectedAnalysisId = "analysis-1"

        var form = createForm()
        var nameField = findRequired(form, "analysisNameField")

        compare(form.isEdit, true)
        compare(nameField.text, "Loaded")
        compare(form.exportFormat, "xlsx")
    }

    function test_updateAndDeleteAnalysisUseCurrentId() {
        session.analysesData = [{
            id: "analysis-2",
            name: "Old",
            type: "plot",
            config: "{}",
            filter: "",
            exportFormat: "png",
            includeCalcAdjustments: true,
            exportState: "{}",
            snapshotTransactions: "[]",
            adjustments: "{}"
        }]
        session.selectedAnalysisId = "analysis-2"

        var form = createForm()
        var nameField = findRequired(form, "analysisNameField")
        var updateButton = findRequired(form, "analysisUpdateButton")
        var deleteButton = findRequired(form, "analysisDeleteButton")

        nameField.text = "Updated"
        updateButton.clicked()
        compare(analysisController.updateCalls, 1)
        compare(analysisController.lastUpdate.id, "analysis-2")
        compare(analysisController.lastUpdate.name, "Updated")

        deleteButton.clicked()
        compare(analysisController.deleteCalls, 1)
        compare(analysisController.lastDeleteId, "analysis-2")
    }

    function test_navigationMovesSelectedAnalysisId() {
        session.analysesData = [
            { id: "analysis-1", name: "A", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" },
            { id: "analysis-2", name: "B", type: "plot", config: "{}", filter: "", exportFormat: "png", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]", adjustments: "{}" }
        ]
        session.selectedAnalysisId = "analysis-1"

        var form = createForm()
        var nextButton = findRequired(form, "analysisNextButton")
        var prevButton = findRequired(form, "analysisPreviousButton")

        nextButton.clicked()
        compare(session.selectedAnalysisId, "analysis-2")

        prevButton.clicked()
        compare(session.selectedAnalysisId, "analysis-1")
    }

    function test_resetAndToggleWorkspace() {
        var form = createForm()
        var toggleButton = findRequired(form, "analysisToggleWorkspaceButton")
        var resetButton = findRequired(form, "analysisResetButton")
        var dateMode = findRequired(form, "analysisDateModeComboBox")
        var yearField = findRequired(form, "analysisYearField")

        dateMode.currentIndex = 0
        yearField.text = "2027"
        toggleButton.clicked()
        compare(form.filterWorkspaceIndex, 1)

        resetButton.clicked()
        compare(form.allocatableMode, "all")
        compare(form.filterWorkspaceIndex, 1)
    }

    function test_filterSelectionAndCalcApply() {
        var form = createForm()
        var propertyCheck = findRequired(form, "analysisPropertyFilterCheckBox")
        var contractTypeCheck = findRequired(form, "analysisContractTypeFilterCheckBox")
        var allocatableCombo = findRequired(form, "analysisAllocatableModeComboBox")
        var txSelectionCheck = findRequired(form, "analysisTransactionSelectionCheckBox")
        var calcNameField = findRequired(form, "analysisCalcNameField")
        var calcPercentField = findRequired(form, "analysisCalcPercentField")
        var applyCalcButton = findRequired(form, "analysisApplyCalcButton")

        propertyCheck.checked = false
        propertyCheck.clicked()
        verify(form.selectedPropertyIds.length < session.propertiesData.length)

        contractTypeCheck.checked = false
        contractTypeCheck.clicked()
        verify(form.selectedContractTypes.length < analysisController.availableContractTypes.length)

        allocatableCombo.currentIndex = 1
        compare(form.allocatableMode, "allocatable")

        txSelectionCheck.checked = true
        txSelectionCheck.clicked()
        calcNameField.text = "VAT"
        calcPercentField.text = "19"
        applyCalcButton.clicked()

        verify(form.pendingAdjustmentsJson.indexOf("selected") !== -1)
    }
}
