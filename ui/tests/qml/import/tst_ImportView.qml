/**
 * @file ui/tests/qml/import/tst_ImportView.qml
 * @brief Provides QML tests for ImportView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ImportViewTests"
    when: windowShown
    width: 960
    height: 640

    property var status: QtObject {
        property string text: ""
    }

    property var settingsViewModel: QtObject {
        property string importDefaultPath: ""
    }

    property var actions: QtObject {
        property int browseCalls: 0
        signal importFileSelected(string path)
        signal importFilesSelected(var paths)
        signal importFileDropped(string path)
        signal importFilesDropped(var paths)
        function browseImportPdf() { browseCalls += 1 }
    }

    property var importWorkflow: QtObject {
        property bool isRunning: false
        property bool isPaused: false
        property real progress: 0
        property string phase: ""
        property string error: ""
        property string selectedFile: ""
        property int queuedCount: 0
        property var queuedFiles: []
        property var draft: null
        property bool hasPrevDraft: false
        property bool hasNextDraft: false
        property bool hasDraftStack: false
        property bool hasPersistedDraftValue: false

        signal stateChanged()
        signal importFileSelected(string path)
        signal importFilesSelected(var paths)
        signal importFileDropped(string path)
        signal importFilesDropped(var paths)
        signal importFinished()
        signal importCanceled()
        signal importFailed(string error)

        property int startCalls: 0
        property int resetCalls: 0
        property int cancelCalls: 0
        property int cancelAllCalls: 0
        property int togglePauseCalls: 0
        property int upsertPauseCalls: 0
        property int clearDraftCalls: 0
        property int persistCalls: 0
        property int clearPersistedCalls: 0
        property int finalizeCalls: 0
        property int prevDraftCalls: 0
        property int nextDraftCalls: 0
        property bool nextDraftClearsDraft: false
        property int runNoteCalls: 0
        property var lastRunNote: ({})
        property string lastClearedDraftId: ""
        property string finalizeResult: "statement-finalized"

        function startStatementImport() { startCalls += 1 }
        function resetStatus() { resetCalls += 1 }
        function cancelImport() { cancelCalls += 1 }
        function cancelAllImports() { cancelAllCalls += 1 }
        function togglePause() { togglePauseCalls += 1; isPaused = !isPaused }
        function clearDraft() { clearDraftCalls += 1; draft = null }
        function openPrevDraft() { prevDraftCalls += 1 }
        function openNextDraft() {
            nextDraftCalls += 1
            if (nextDraftClearsDraft) {
                draft = null
                stateChanged()
            }
        }
        function hasPersistedDraft() { return hasPersistedDraftValue }
        function persistStatementDraft(draft) { persistCalls += 1 }
        function clearPersistedStatementDraft(draftId) {
            clearPersistedCalls += 1
            lastClearedDraftId = draftId
        }
        function finalizeStatementDraft(draft) {
            finalizeCalls += 1
            return finalizeResult
        }
        function syncCurrentTransactionDraft(draft) {}
        function currentTransactionViewState(draft) { return ({ actorChoices: [], effectiveAllocatable: false }) }
        function addRunNote(statusText, messageText, draftAttached, statementId, contextDraftId) {
            runNoteCalls += 1
            lastRunNote = {
                status: statusText,
                message: messageText,
                draftAttached: draftAttached,
                statementId: statementId || "",
                contextDraftId: contextDraftId || ""
            }
        }
    }

    property var navigation: QtObject {
        property int sectionValue: -1
        function setSectionValue(value) { sectionValue = value }
    }

    property var appContext: QtObject {
        property var importWorkflow: testCase.importWorkflow
        property var settingsViewModel: testCase.settingsViewModel
        property var actions: testCase.actions
        property var status: testCase.status
        property var navigation: testCase.navigation
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacing: 8
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property int viewNavigationButtonWidth: 42
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property int chartValueLabelWidth: 120
        property int controlHeight: 32
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
        property color success: "#008800"
        property int viewSectionIconSize: 42
    }

    Component {
        id: importViewComponent

        ImportView {
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

    function createDraftObject() {
        return Qt.createQmlObject('import QtQml 2.15; QtObject { property string draftId: "draft-1"; property string name: "Draft"; property int currentIndex: 0; property int count: 2; property var current: ({ id: "tx-1" }); property var transactions: QtObject { function setStatus(index, value) {} function setActorText(index, text) {} function setActorId(index, id) {} function setNewActorSelected(index, selected) {} function setType(index, value) {} function setContractId(index, id) {} function setNewContractSelected(index, selected) {} function setBookingDate(index, value) {} function setValuta(index, value) {} function setName(index, value) {} function setAllocatable(index, value) {} function setAllocatableManualOverride(index, value) {} function removeTransaction(index) {} } function prev() { if (currentIndex > 0) currentIndex -= 1 } function next() { if (currentIndex < count - 1) currentIndex += 1 } function removeTransaction(index) { if (count > 1) count -= 1 } function refresh() {} }', testCase)
    }

    function createView() {
        return createTemporaryObject(importViewComponent, testCase)
    }

    function init() {
        status.text = ""
        settingsViewModel.importDefaultPath = ""
        actions.browseCalls = 0

        importWorkflow.isRunning = false
        importWorkflow.selectedFile = ""
        importWorkflow.queuedCount = 0
        importWorkflow.queuedFiles = []
        importWorkflow.draft = null
        importWorkflow.hasPrevDraft = false
        importWorkflow.hasNextDraft = false
        importWorkflow.hasDraftStack = false
        importWorkflow.hasPersistedDraftValue = false
        importWorkflow.startCalls = 0
        importWorkflow.resetCalls = 0
        importWorkflow.cancelCalls = 0
        importWorkflow.cancelAllCalls = 0
        importWorkflow.togglePauseCalls = 0
        importWorkflow.upsertPauseCalls = 0
        importWorkflow.isPaused = false
        importWorkflow.clearDraftCalls = 0
        importWorkflow.persistCalls = 0
        importWorkflow.clearPersistedCalls = 0
        importWorkflow.finalizeCalls = 0
        importWorkflow.prevDraftCalls = 0
        importWorkflow.nextDraftCalls = 0
        importWorkflow.nextDraftClearsDraft = false
        importWorkflow.runNoteCalls = 0
        importWorkflow.lastRunNote = ({})
        importWorkflow.lastClearedDraftId = ""
        importWorkflow.finalizeResult = "statement-finalized"

        navigation.sectionValue = -1
    }

    function test_IMP_V_001_defaultImportSelectionUsesSettingsPath() {
        settingsViewModel.importDefaultPath = "test:///import/default.pdf"

        var view = createView()
        wait(0)

        compare(importWorkflow.selectedFile, "test:///import/default.pdf")
    }

    function test_IMP_V_002_startButtonCallsController() {
        importWorkflow.selectedFile = "test:///import/statement.pdf"

        var view = createView()
        wait(0)

        var startButton = findRequired(view, "importStartButton")

        startButton.clicked()

        compare(importWorkflow.startCalls, 1)
    }

    function test_IMP_V_003_clearButtonCallsController() {
        importWorkflow.selectedFile = "test:///import/statement.pdf"

        var view = createView()
        wait(0)

        var clearButton = findRequired(view, "importClearButton")

        clearButton.clicked()

        compare(importWorkflow.resetCalls, 1)
    }

    function test_IMP_V_004_cancelButtonCallsController() {
        importWorkflow.isRunning = true
        importWorkflow.queuedCount = 2

        var view = createView()
        wait(0)

        var cancelButton = findRequired(view, "importCancelButton")

        cancelButton.clicked()

        compare(importWorkflow.cancelCalls, 1)
    }

    function test_IMP_V_005_cancelAllButtonCallsController() {
        importWorkflow.isRunning = true
        importWorkflow.queuedCount = 2

        var view = createView()
        wait(0)

        var cancelAllButton = findRequired(view, "importCancelAllButton")

        cancelAllButton.clicked()

        compare(importWorkflow.cancelAllCalls, 1)
    }

    function test_IMP_V_007_pauseButtonCallsControllerAndShowsResume() {
        importWorkflow.isRunning = true

        var view = createView()
        wait(0)

        var pauseButton = findRequired(view, "importPauseButton")

        compare(pauseButton.text, "Pause")
        pauseButton.clicked()

        compare(importWorkflow.togglePauseCalls, 1)
        compare(pauseButton.text, "Resume")
    }

    function test_IMP_V_008_runningButtonOrderKeepsPauseAtRightEdge() {
        importWorkflow.isRunning = true
        importWorkflow.queuedCount = 2

        var view = createView()
        wait(0)

        var cancelButton = findRequired(view, "importCancelButton")
        var pauseButton = findRequired(view, "importPauseButton")
        var cancelAllButton = findRequired(view, "importCancelAllButton")

        verify(cancelButton.x < cancelAllButton.x)
        verify(cancelAllButton.x < pauseButton.x)
    }

    function test_IMP_V_009_importPageDraftNavigationButtonsStayAtOuterEdges() {
        importWorkflow.hasDraftStack = true
        importWorkflow.selectedFile = "test:///import/statement.pdf"

        var view = createView()
        wait(0)

        var prevDraftButton = findRequired(view, "importPreviousDraftButton")
        var clearButton = findRequired(view, "importClearButton")
        var startButton = findRequired(view, "importStartButton")
        var nextDraftButton = findRequired(view, "importNextDraftButton")

        compare(prevDraftButton.enabled, true)
        compare(nextDraftButton.enabled, true)
        verify(prevDraftButton.x < clearButton.x)
        verify(startButton.x < nextDraftButton.x)

        prevDraftButton.clicked()
        nextDraftButton.clicked()

        compare(importWorkflow.prevDraftCalls, 1)
        compare(importWorkflow.nextDraftCalls, 1)
    }

    function test_IMP_V_010_runningDraftNavigationButtonsStayAtOuterEdges() {
        importWorkflow.isRunning = true
        importWorkflow.queuedCount = 2
        importWorkflow.hasDraftStack = true

        var view = createView()
        wait(0)

        var prevDraftButton = findRequired(view, "importPreviousDraftButton")
        var cancelButton = findRequired(view, "importCancelButton")
        var pauseButton = findRequired(view, "importPauseButton")
        var nextDraftButton = findRequired(view, "importNextDraftButton")

        compare(prevDraftButton.enabled, true)
        compare(nextDraftButton.enabled, true)
        verify(prevDraftButton.x < cancelButton.x)
        verify(pauseButton.x < nextDraftButton.x)
    }

    function test_IMP_V_011_importPageDraftNavigationDisabledWithoutDraftLogs() {
        importWorkflow.hasPersistedDraftValue = true
        importWorkflow.selectedFile = "test:///import/statement.pdf"

        var view = createView()
        wait(0)

        compare(findRequired(view, "importPreviousDraftButton").enabled, false)
        compare(findRequired(view, "importNextDraftButton").enabled, false)
    }

    function test_IMP_V_012_draftPageOuterNavigationCanReturnToImportHome() {
        importWorkflow.draft = createDraftObject()
        importWorkflow.hasDraftStack = true
        importWorkflow.hasNextDraft = true

        var view = createView()
        wait(0)

        var stack = findRequired(view, "importContentStack")
        compare(stack.currentIndex, 1)

        importWorkflow.nextDraftClearsDraft = true

        findRequired(view, "statementDraftNextPageButton").clicked()
        compare(importWorkflow.nextDraftCalls, 1)
        tryCompare(stack, "currentIndex", 0)
    }

    function test_IMP_V_006_draftStateSwitchesToDraftPage() {
        importWorkflow.draft = createDraftObject()

        var view = createView()
        wait(0)

        var stack = findRequired(view, "importContentStack")
        compare(stack.currentIndex, 1)
    }

    function test_IMP_D_005_transactionNavigationButtonsUpdateCurrentIndex() {
        importWorkflow.draft = createDraftObject()

        var view = createView()
        wait(0)

        var nextTransactionButton = findRequired(view, "statementDraftNextTransactionButton")
        var prevTransactionButton = findRequired(view, "statementDraftPrevTransactionButton")

        nextTransactionButton.clicked()
        compare(importWorkflow.draft.currentIndex, 1)

        prevTransactionButton.clicked()
        compare(importWorkflow.draft.currentIndex, 0)
    }
}
