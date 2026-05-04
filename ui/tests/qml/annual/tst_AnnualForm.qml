/**
 * @file ui/tests/qml/annual/tst_AnnualForm.qml
 * @brief Provides QML tests for AnnualForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnnualFormTests"
    when: windowShown
    width: 960
    height: 640

    property var annualById: ({})
    property var liveTransactions: []

    property var session: QtObject {
        property var selectedAnnual: null
        property string selectedAnnualId: ""
        property var annuals: []
        property var analysesData: []

        function annualRows() { return annuals || [] }
        function analysisRows() { return analysesData || [] }

        function navigatedId(rows, currentId, delta, fallbackIndex) {
            var list = rows || []
            if (list.length === 0)
                return ""
            var idx = -1
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(currentId || "")) {
                    idx = i
                    break
                }
            }
            if (idx < 0)
                idx = fallbackIndex
            else
                idx = (idx + delta + list.length) % list.length
            return String(list[idx].id || "")
        }

        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) {
            var list = rows || []
            var kept = []
            for (var i = 0; i < list.length; ++i) {
                var rowId = String(list[i][key] || "")
                if (rowId !== String(removedId || ""))
                    kept.push(list[i])
            }
            if (kept.length === 0)
                return ""
            var idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1))
            return String(kept[idx][key] || "")
        }
    }

    property var annualController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
        }

        function annual(id) {
            return testCase.annualById[String(id || "")] || ({ id: "", name: "", year: 0, transactionIds: [], assignedAnalysisIds: [] })
        }

        function saveAnnual(id, name, year, assignedAnalysisIds) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                year: year,
                assignedAnalysisIds: assignedAnalysisIds
            }
            return id && id.length > 0 ? id : "annual-new"
        }

        function deleteAnnual(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var analysisController: QtObject {
        property int updateCalls: 0
        property var lastUpdate: ({})

        function reset() {
            updateCalls = 0
            lastUpdate = ({})
        }

        function updateAnalysis(id, name, type, config, filter, exportFormat, includeCalcAdjustments, exportState, snapshotTransactions) {
            updateCalls += 1
            lastUpdate = {
                id: id,
                name: name,
                type: type,
                exportFormat: exportFormat
            }
        }
    }

    property var transactionController: QtObject {
        function transaction(id) {
            var list = testCase.liveTransactions || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return list[i]
            }
            return ({})
        }

        function transactions() {
            return testCase.liveTransactions || []
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var annualController: testCase.annualController
        property var analysisController: testCase.analysisController
        property var transactionController: testCase.transactionController
        property bool isDebugBuild: false
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacingSmall: 6
        property int spacing: 8
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
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
        id: annualFormComponent
        AnnualForm {
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

    function createAnnualObject(source) {
        if (!source)
            return null
        var annualObject = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: "" }', testCase)
        annualObject.id = source.id || ""
        return annualObject
    }

    function createForm(selectedAnnual) {
        var annualObject = createAnnualObject(selectedAnnual)
        session.selectedAnnual = annualObject
        session.selectedAnnualId = annualObject ? annualObject.id : ""
        return createTemporaryObject(annualFormComponent, testCase)
    }

    function init() {
        annualById = ({})
        liveTransactions = []
        annualController.reset()
        analysisController.reset()
        session.selectedAnnual = null
        session.selectedAnnualId = ""
        session.annuals = []
        session.analysesData = []
    }

    function test_createModeSavesAnnualAndSelectsId() {
        var form = createForm(null)
        var nameField = findRequired(form, "annualNameField")
        var yearField = findRequired(form, "annualYearField")

        nameField.text = "Y2026"
        form.annualNameText = "Y2026"
        yearField.text = "2026"
        form.yearText = "2026"
        form.submitAnnual()

        compare(annualController.saveCalls, 1)
        compare(annualController.lastSave.id, "")
        compare(annualController.lastSave.year, 2026)
        compare(session.selectedAnnualId, "annual-new")
    }

    function test_readModeLoadsAnnualState() {
        annualById["annual-1"] = {
            id: "annual-1",
            name: "Loaded",
            year: 2027,
            transactionIds: ["tx-1"],
            assignedAnalysisIds: ["analysis-1"]
        }
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "tab", config: "{}", filter: "", exportFormat: "xlsx", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]" }
        ]

        var form = createForm({ id: "annual-1" })
        var nameField = findRequired(form, "annualNameField")
        var yearField = findRequired(form, "annualYearField")

        compare(form.isEdit, true)
        compare(nameField.text, "Loaded")
        compare(yearField.text, "2027")
        compare(form.assignedAnalysisIds.length, 1)
        compare(form.assignedAnalysisIds[0], "analysis-1")
    }

    function test_updateModeSavesCurrentAnnualId() {
        annualById["annual-2"] = {
            id: "annual-2",
            name: "Old",
            year: 2025,
            transactionIds: [],
            assignedAnalysisIds: []
        }

        var form = createForm({ id: "annual-2" })
        var yearField = findRequired(form, "annualYearField")

        yearField.text = "2026"
        form.yearText = "2026"
        form.submitAnnual()

        compare(annualController.saveCalls, 1)
        compare(annualController.lastSave.id, "annual-2")
        compare(annualController.lastSave.year, 2026)
    }

    function test_yearValidationDisablesCreateWhenInvalid() {
        var form = createForm(null)
        form.yearText = ""
        compare(form.canSubmit(), false)

        form.yearText = "2026"
        compare(form.canSubmit(), true)
    }

    function test_analysisAssignmentAndRemovalUpdatesIds() {
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "tab", config: "{}", filter: "", exportFormat: "xlsx", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]" }
        ]

        var form = createForm(null)
        var addAnalysisCombo = findRequired(form, "annualAddAnalysisComboBox")
        var addAnalysisButton = findRequired(form, "annualAddAnalysisButton")

        addAnalysisCombo.currentIndex = 0
        addAnalysisButton.clicked()

        compare(form.assignedAnalysisIds.length, 1)
        compare(form.assignedAnalysisIds[0], "analysis-1")

        var removeButton = findRequired(form, "annualRemoveAnalysisButton")
        removeButton.clicked()
        compare(form.assignedAnalysisIds.length, 0)
    }

    function test_analysisExportFormatUpdateCallsController() {
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "tab", config: "{}", filter: "", exportFormat: "xlsx", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]" }
        ]

        var form = createForm(null)
        var addAnalysisCombo = findRequired(form, "annualAddAnalysisComboBox")
        var addAnalysisButton = findRequired(form, "annualAddAnalysisButton")

        addAnalysisCombo.currentIndex = 0
        addAnalysisButton.clicked()

        var exportFormatCombo = findRequired(form, "annualAnalysisExportFormatComboBox")
        exportFormatCombo.currentIndex = 1
        exportFormatCombo.activated(1)

        compare(analysisController.updateCalls, 1)
        compare(analysisController.lastUpdate.id, "analysis-1")
        compare(analysisController.lastUpdate.exportFormat, "csv")
    }

    function test_navigationButtonsMoveSelectionId() {
        session.annuals = [
            { id: "annual-1", name: "A" },
            { id: "annual-2", name: "B" }
        ]

        var form = createForm({ id: "annual-1" })
        var nextButton = findRequired(form, "annualNextButton")
        var previousButton = findRequired(form, "annualPreviousButton")

        nextButton.clicked()
        compare(session.selectedAnnualId, "annual-2")

        previousButton.clicked()
        compare(session.selectedAnnualId, "annual-1")
    }

    function test_deleteButtonDeletesCurrentAnnual() {
        annualById["annual-3"] = {
            id: "annual-3",
            name: "DeleteMe",
            year: 2024,
            transactionIds: [],
            assignedAnalysisIds: []
        }

        var form = createForm({ id: "annual-3" })
        var deleteButton = findRequired(form, "annualDeleteButton")

        deleteButton.clicked()

        compare(annualController.deleteCalls, 1)
        compare(annualController.lastDeleteId, "annual-3")
    }
}
