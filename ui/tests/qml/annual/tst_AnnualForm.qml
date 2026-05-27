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
        property int dataRevision: 0
        property var annuals: []
        property var analysesData: []

        function annualRows() { return annuals || [] }
        function analysisRows() { return analysesData || [] }

        function indexOfId(rows, id) {
            var list = rows || []
            var target = String(id || "")
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === target)
                    return i
            }
            return -1
        }

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
            return testCase.annualById[String(id || "")] || ({ id: "", name: "", year: 0, transactionIds: [], analysisIds: [] })
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

    property var workspaceFacade: QtObject {
        property bool previewReturnsLegacyOnlyTransactions: false
        function annualRows() { return session.annualRows() }
        function analysisRows() { return session.analysisRows() }
        function analysisIdFromRow(row) {
            return row && row.id ? String(row.id) : ""
        }
        function normalizeAnalysisIds(values) {
            var list = []
            var seen = ({})
            var source = values
            if (!Array.isArray(source))
                source = source !== undefined && source !== null ? [source] : []
            for (var i = 0; i < source.length; ++i) {
                var value = source[i]
                var id = ""
                if (value && typeof value === "object")
                    id = value.id ? String(value.id) : ""
                else
                    id = String(value || "")
                if (id.length === 0 || seen[id])
                    continue
                seen[id] = true
                list.push(id)
            }
            return list
        }
        function assignedAnnualAnalysisRows(allRows, selectedIds) {
            var rows = allRows || []
            var ids = normalizeAnalysisIds(selectedIds)
            var byId = ({})
            for (var i = 0; i < rows.length; ++i) {
                var row = rows[i] || ({})
                var id = row.id ? String(row.id) : ""
                if (id.length === 0)
                    continue
                byId[id] = {
                    id: id,
                    name: row.name ? String(row.name) : "",
                    display: row.display ? String(row.display) : (row.name ? String(row.name) : id),
                    type: row.type ? String(row.type) : "tab",
                    config: row.config ? String(row.config) : "{}",
                    filter: row.filter ? String(row.filter) : "",
                    exportFormat: row.exportFormat ? String(row.exportFormat) : "",
                    includeCalcAdjustments: row.includeCalcAdjustments !== undefined ? !!row.includeCalcAdjustments : true,
                    exportState: row.exportState ? String(row.exportState) : "{}",
                    snapshotTransactions: row.snapshotTransactions ? String(row.snapshotTransactions) : "[]"
                }
            }
            var out = []
            for (var j = 0; j < ids.length; ++j) {
                if (byId[ids[j]])
                    out.push(byId[ids[j]])
            }
            return out
        }
        function availableAnnualAnalysisRows(allRows, selectedIds) {
            var rows = assignedAnnualAnalysisRows(allRows, normalizeAnalysisIds(allRows))
            var selected = normalizeAnalysisIds(selectedIds)
            var selectedSet = ({})
            for (var i = 0; i < selected.length; ++i)
                selectedSet[selected[i]] = true
            var out = []
            for (var j = 0; j < rows.length; ++j) {
                if (!selectedSet[String(rows[j].id)])
                    out.push(rows[j])
            }
            return out
        }
        function deriveAnnualState(allAnalysisRows, selectedIds, liveTransactions, contractRows, year) {
            var assigned = assignedAnnualAnalysisRows(allAnalysisRows, selectedIds)
            var snapshotById = ({})
            var dup = ({})
            for (var i = 0; i < assigned.length; ++i) {
                var text = String(assigned[i].snapshotTransactions || "[]")
                var parsed = []
                try {
                    var doc = JSON.parse(text)
                    if (Array.isArray(doc))
                        parsed = doc
                    else if (doc && Array.isArray(doc.transactions))
                        parsed = doc.transactions
                } catch (e) {}
                for (var j = 0; j < parsed.length; ++j) {
                    var snap = parsed[j] || ({})
                    var txId = String(snap.id || snap.transactionId || "")
                    if (txId.length === 0)
                        continue
                    dup[txId] = (dup[txId] || 0) + 1
                    if (!snapshotById[txId])
                        snapshotById[txId] = snap
                }
            }
            var liveById = ({})
            var live = liveTransactions || []
            for (var k = 0; k < live.length; ++k) {
                var tx = live[k] || ({})
                if (tx.id)
                    liveById[String(tx.id)] = tx
            }
            var metrics = ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
            var issues = ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
            var rows = []
            var ids = Object.keys(snapshotById)
            for (var x = 0; x < ids.length; ++x) {
                var id = ids[x]
                var s = snapshotById[id]
                var l = liveById[id] || null
                var hasLive = !!l
                var bookingDate = hasLive ? String(l.bookingDate || "") : String(s.bookingDate || s.date || "")
                var by = bookingDate.length >= 4 ? parseInt(bookingDate.substring(0, 4)) : -1
                var status = hasLive ? Number(l.status || 0) : 0
                if (year > 0 && by > 0 && by !== year)
                    issues.mixedInAnnual += 1
                if ((dup[id] || 0) > 1)
                    issues.duplicateCount += 1
                if (!hasLive)
                    issues.missingLive += 1
                if (status === 1) metrics.unverified += 1
                else if (status === 2) metrics.verified += 1
                else if (status === 3) metrics.completed += 1
                else metrics.neutral += 1
                rows.push({
                    id: id,
                    name: hasLive ? String(l.name || "") : String(s.transactionName || s.name || ""),
                    bookingDate: bookingDate,
                    amount: hasLive ? Number(l.amount || 0) : Number(s.amount || 0),
                    status: status,
                    statusText: status === 1 ? "Unverified" : (status === 2 ? "Verified" : (status === 3 ? "Completed" : "Neutral")),
                    duplicateCount: dup[id] || 1,
                    isDuplicate: (dup[id] || 0) > 1,
                    isMixedYear: year > 0 && by > 0 && by !== year,
                    isMissingLive: !hasLive,
                    statementId: hasLive ? String(l.statementId || "") : String(s.statementId || ""),
                    actorId: hasLive ? String(l.actorId || "") : "",
                    allocatable: hasLive ? !!l.allocatable : !!s.allocatable,
                    description: hasLive ? String(l.description || "") : "",
                    propertyIds: hasLive ? (l.propertyIds || []) : []
                })
            }
            for (var y = 0; y < live.length; ++y) {
                var t = live[y] || ({})
                var lid = String(t.id || "")
                if (lid.length === 0 || snapshotById[lid])
                    continue
                var lyear = String(t.bookingDate || "").length >= 4 ? parseInt(String(t.bookingDate).substring(0, 4)) : -1
                if (year > 0 && lyear === year)
                    issues.missingFromYear += 1
            }
            return { issues: issues, statusMetrics: metrics, transactions: rows }
        }
        function annualResultState(annualId) {
            var annual = testCase.annualController.annual(annualId)
            var derived = deriveAnnualState(session.analysisRows(), annual.analysisIds || [], testCase.transactionController.transactions(), [], Number(annual.year || 0))
            var stats = {
                assignedAnalysisCount: (annual.analysisIds || []).length,
                snapshotTransactionCount: derived.transactions ? derived.transactions.length : 0,
                missingFromYear: derived.issues ? derived.issues.missingFromYear : 0,
                mixedYear: derived.issues ? derived.issues.mixedInAnnual : 0,
                duplicateCount: derived.issues ? derived.issues.duplicateCount : 0,
                missingLive: derived.issues ? derived.issues.missingLive : 0,
                neutral: derived.statusMetrics ? derived.statusMetrics.neutral : 0,
                unverified: derived.statusMetrics ? derived.statusMetrics.unverified : 0,
                verified: derived.statusMetrics ? derived.statusMetrics.verified : 0,
                completed: derived.statusMetrics ? derived.statusMetrics.completed : 0
            }
            return {
                annualId: String(annualId || ""),
                annualName: String(annual.name || ""),
                year: Number(annual.year || 0),
                stats: stats,
                transactions: derived.transactions || []
            }
        }
        function annualResultStatePreview(annualId, selectedIds, year) {
            var annual = testCase.annualController.annual(annualId)
            var ids = selectedIds || (annual.analysisIds || [])
            var targetYear = Number(year || annual.year || 0)
            var derived = deriveAnnualState(session.analysisRows(), ids, testCase.transactionController.transactions(), [], targetYear)
            var stats = {
                assignedAnalysisCount: (ids || []).length,
                snapshotTransactionCount: derived.transactions ? derived.transactions.length : 0,
                missingFromYear: derived.issues ? derived.issues.missingFromYear : 0,
                mixedYear: derived.issues ? derived.issues.mixedInAnnual : 0,
                duplicateCount: derived.issues ? derived.issues.duplicateCount : 0,
                missingLive: derived.issues ? derived.issues.missingLive : 0,
                neutral: derived.statusMetrics ? derived.statusMetrics.neutral : 0,
                unverified: derived.statusMetrics ? derived.statusMetrics.unverified : 0,
                verified: derived.statusMetrics ? derived.statusMetrics.verified : 0,
                completed: derived.statusMetrics ? derived.statusMetrics.completed : 0
            }
            var payload = {
                annualId: String(annualId || ""),
                annualName: String(annual.name || ""),
                year: targetYear,
                stats: stats,
                transactions: derived.transactions || []
            }
            if (previewReturnsLegacyOnlyTransactions) {
                if (!payload.transactions || payload.transactions.length === 0) {
                    payload.transactions = [{
                        key: "live|legacy-live-1",
                        id: "legacy-live-1",
                        name: "Legacy Live Row",
                        bookingDate: "2026-06-01",
                        amount: 77.0,
                        status: 2,
                        statusText: "Verified",
                        allocatable: true
                    }]
                } else {
                    payload.transactions[0].key = "live|" + String(payload.transactions[0].id || "legacy-live-1")
                }
                return payload
            }
            payload.deduplicated = []
            payload.similar = []
            payload.divergent = []
            payload.workspaceOnly = []
            for (var i = 0; i < payload.transactions.length; ++i) {
                var row = payload.transactions[i] || ({})
                var key = String(row.key || "")
                if (key.indexOf("live|") === 0)
                    payload.workspaceOnly.push(row)
                else
                    payload.deduplicated.push(row)
            }
            return payload
        }
        function transaction(id) { return testCase.transactionController.transaction(id) }
        function transactions() { return testCase.transactionController.transactions() }
        function transactionRows() { return testCase.transactionController.transactions() }
        function annual(id) { return testCase.annualController.annual(id) }
        function saveAnnual(id, name, year, assignedAnalysisIds) {
            return testCase.annualController.saveAnnual(id, name, year, assignedAnalysisIds)
        }
        function deleteAnnual(id) { testCase.annualController.deleteAnnual(id) }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var sessionState: testCase.session
        property var workspaceFacade: testCase.workspaceFacade
        property var analysisWorkflow: testCase.analysisController
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
        workspaceFacade.previewReturnsLegacyOnlyTransactions = false
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
            analysisIds: ["analysis-1"]
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
        compare(form.analysisIds.length, 1)
        compare(form.analysisIds[0], "analysis-1")
    }

    function test_updateModeSavesCurrentAnnualId() {
        annualById["annual-2"] = {
            id: "annual-2",
            name: "Old",
            year: 2025,
            transactionIds: [],
            analysisIds: []
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

        compare(form.analysisIds.length, 1)
        compare(form.analysisIds[0], "analysis-1")
        compare(form.assignedAnalysisRows.length, 1)
        compare(form.assignedAnalysisRows[0].id, "analysis-1")

        var removeButton = findRequired(form, "annualRemoveAnalysisButton")
        removeButton.clicked()
        compare(form.analysisIds.length, 0)
    }

    function test_readModeRendersAssignedAnalysisRows() {
        annualById["annual-4"] = {
            id: "annual-4",
            name: "WithAssigned",
            year: 2026,
            transactionIds: [],
            analysisIds: ["analysis-1"]
        }
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "tab", config: "{}", filter: "", exportFormat: "xlsx", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]" }
        ]

        var form = createForm({ id: "annual-4" })
        compare(form.assignedAnalysisRows.length, 1)
        compare(form.assignedAnalysisRows[0].id, "analysis-1")
        verify(findRequired(form, "annualRemoveAnalysisButton") !== null)
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

    function test_analysisExportFormatChangeEnablesAnnualUpdate() {
        annualById["annual-6"] = {
            id: "annual-6",
            name: "WithAnalysis",
            year: 2026,
            transactionIds: [],
            analysisIds: ["analysis-1"]
        }
        session.analysesData = [
            { id: "analysis-1", name: "A1", type: "plot", config: "{}", filter: "", exportFormat: "jpg", includeCalcAdjustments: true, exportState: "{}", snapshotTransactions: "[]" }
        ]

        var form = createForm({ id: "annual-6" })
        var updateButton = findRequired(form, "annualUpdateButton")
        var exportFormatCombo = findRequired(form, "annualAnalysisExportFormatComboBox")

        compare(updateButton.enabled, false)
        exportFormatCombo.currentIndex = 0
        exportFormatCombo.activated(0)
        compare(analysisController.updateCalls, 1)
        compare(updateButton.enabled, true)
    }

    function test_assignedAnalysesBuildAnnualTransactionOverview() {
        annualById["annual-5"] = {
            id: "annual-5",
            name: "Overview",
            year: 2026,
            transactionIds: [],
            analysisIds: ["analysis-1", "analysis-2"]
        }
        session.analysesData = [
            {
                id: "analysis-1",
                name: "A1",
                type: "tab",
                config: "{}",
                filter: "",
                exportFormat: "xlsx",
                includeCalcAdjustments: true,
                exportState: "{}",
                snapshotTransactions: "[{\"id\":\"tx-1\",\"transactionName\":\"Rent\",\"date\":\"2026-01-15\",\"amount\":100,\"allocatable\":true},{\"id\":\"tx-2\",\"transactionName\":\"Fee\",\"date\":\"2026-01-20\",\"amount\":20,\"allocatable\":false}]"
            },
            {
                id: "analysis-2",
                name: "A2",
                type: "tab",
                config: "{}",
                filter: "",
                exportFormat: "xlsx",
                includeCalcAdjustments: true,
                exportState: "{}",
                snapshotTransactions: "[{\"id\":\"tx-1\",\"transactionName\":\"Rent\",\"date\":\"2026-01-15\",\"amount\":100,\"allocatable\":true}]"
            }
        ]
        liveTransactions = [
            { id: "tx-1", name: "Rent", bookingDate: "2026-01-15", amount: 100, status: 2, statementId: "st-1", allocatable: true, propertyIds: [] }
        ]

        var form = createForm({ id: "annual-5" })
        compare(form.annualTransactions.length, 2)
        compare(form.annualVerificationIssues.duplicateCount, 1)
        compare(form.annualVerificationIssues.missingLive, 1)
        compare(form.annualStatusMetrics.verified, 1)
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

    function test_selectionChangeReloadsFormFields() {
        annualById["annual-1"] = { id: "annual-1", name: "First", year: 2024, transactionIds: [], analysisIds: [] }
        annualById["annual-2"] = { id: "annual-2", name: "Second", year: 2025, transactionIds: [], analysisIds: [] }

        var form = createForm({ id: "annual-1" })
        var nameField = findRequired(form, "annualNameField")
        compare(nameField.text, "First")

        session.selectedAnnualId = "annual-2"
        wait(0)
        compare(nameField.text, "Second")
    }

    function test_deleteButtonDeletesCurrentAnnual() {
        annualById["annual-3"] = {
            id: "annual-3",
            name: "DeleteMe",
            year: 2024,
            transactionIds: [],
            analysisIds: []
        }

        var form = createForm({ id: "annual-3" })
        var deleteButton = findRequired(form, "annualDeleteButton")

        deleteButton.clicked()

        compare(annualController.deleteCalls, 1)
        compare(annualController.lastDeleteId, "annual-3")
    }

    function test_legacyTransactionsOnlyPayloadStillBuildsWorkspaceOnlyCategory() {
        annualById["annual-7"] = {
            id: "annual-7",
            name: "LegacyPreview",
            year: 2026,
            transactionIds: [],
            analysisIds: ["analysis-1"]
        }
        session.analysesData = [
            {
                id: "analysis-1",
                name: "A1",
                type: "tab",
                config: "{}",
                filter: "",
                exportFormat: "xlsx",
                includeCalcAdjustments: true,
                exportState: "{}",
                snapshotTransactions: "[]"
            }
        ]
        liveTransactions = [
            { id: "live-1", name: "Live Row", bookingDate: "2026-06-01", amount: 77.0, status: 2, statementId: "st-1", allocatable: true, propertyIds: [] }
        ]
        workspaceFacade.previewReturnsLegacyOnlyTransactions = true

        var form = createForm({ id: "annual-7" })
        compare(form.annualTransactions.length, 1)
        compare(form.annualTransactionGroups.workspaceOnly.length, 1)
    }

    function test_yearPickerHandlesRepeatedMixedStepChanges() {
        annualById["annual-8"] = {
            id: "annual-8",
            name: "YearStress",
            year: 2026,
            transactionIds: [],
            analysisIds: ["analysis-1"]
        }
        session.analysesData = [
            {
                id: "analysis-1",
                name: "A1",
                type: "tab",
                config: "{}",
                filter: "",
                exportFormat: "xlsx",
                includeCalcAdjustments: true,
                exportState: "{}",
                snapshotTransactions: "[{\"id\":\"tx-1\",\"transactionName\":\"Rent\",\"date\":\"2026-01-15\",\"amount\":100,\"allocatable\":true}]"
            }
        ]
        liveTransactions = [
            { id: "live-2025", name: "Live2025", bookingDate: "2025-02-01", amount: 50, status: 2, statementId: "st-1", allocatable: true, propertyIds: [] },
            { id: "live-2026", name: "Live2026", bookingDate: "2026-03-01", amount: 60, status: 2, statementId: "st-2", allocatable: true, propertyIds: [] }
        ]

        var form = createForm({ id: "annual-8" })
        var yearDecreaseButton = findRequired(form, "annualYearDecreaseButton")
        var yearIncreaseButton = findRequired(form, "annualYearIncreaseButton")
        compare(form.yearText, "2026")

        for (var i = 0; i < 2; ++i)
            yearIncreaseButton.clicked()
        compare(form.yearText, "2028")

        for (var j = 0; j < 3; ++j)
            yearDecreaseButton.clicked()
        compare(form.yearText, "2025")

        yearIncreaseButton.clicked()
        compare(form.yearText, "2026")

        // Ensure annual state is still recomputed after mixed stepping.
        compare(form.annualVerificationIssues.missingFromYear, 0)
    }
}
