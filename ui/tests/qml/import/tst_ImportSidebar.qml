/**
 * @file ui/tests/qml/import/tst_ImportSidebar.qml
 * @brief Provides QML tests for ImportSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ImportSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var importWorkflow: QtObject {
        property var runs: []
        property int activateCalls: 0
        property int openDraftCalls: 0
        property string lastOpenedDraftId: ""
        property int removeCalls: 0
        property int clearDraftCalls: 0
        property var draft: null
        function activateRunAt(index) { activateCalls += 1 }
        function openPersistedDraft(logId) { openDraftCalls += 1; lastOpenedDraftId = logId }
        function removeRunAt(index) { removeCalls += 1 }
        function clearPersistedStatementDraft(draftId) { clearDraftCalls += 1 }
        function clearDraft() {}
    }

    property var navigation: QtObject {
        property int sectionValue: 0
        property int bookingViewValue: 0
        function setSectionValue(value) { sectionValue = value }
        function setBookingViewValue(value) { bookingViewValue = value }
    }

    property var session: QtObject {
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
    }

    property var appContext: QtObject {
        property var importWorkflow: testCase.importWorkflow
        property var navigation: testCase.navigation
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int borderWidthThin: 1
        property int radius: 3
        property int viewCompactActionButtonSize: 28
        property color accent: "#3366ff"
        property color selectionHighlight: "#aaccee"
        property color border: "#cccccc"
        property color success: "#008800"
        property color warning: "#aa8800"
        property color danger: "#aa0000"
        property color textMuted: "#666666"
    }

    Component {
        id: importSidebarComponent
        ImportSidebar {
            width: 960
            height: 640
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
        return createTemporaryObject(importSidebarComponent, testCase)
    }

    function init() {
        importWorkflow.runs = [
            { logId: "import-1", time: "2026-05-16 10:00:00", status: "Success", file: "/tmp/import.pdf", message: "done", payload: "{\"items\":[]}", draftAttached: false, draftId: "", statementId: "statement-1" }
        ]
        importWorkflow.activateCalls = 0
        importWorkflow.openDraftCalls = 0
        importWorkflow.removeCalls = 0
        importWorkflow.clearDraftCalls = 0
        importWorkflow.lastOpenedDraftId = ""
        importWorkflow.draft = null
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
        navigation.sectionValue = 0
        navigation.bookingViewValue = 0
    }

    function test_runListIsBoundToImportedRuns() {
        var view = createView()
        var runList = findRequired(view, "runLogList")

        compare(runList.count, 1)
    }

    function test_draftRunClickOpensPersistedDraftByDraftId() {
        importWorkflow.runs = [
            { logId: "import-1", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", draftAttached: true, draftId: "draft-1", statementId: "" }
        ]

        var view = createView()
        var runMouse = findRequired(view, "runLogRow_import-1")

        verify(runMouse.enabled)
        runMouse.activateRun()

        compare(importWorkflow.activateCalls, 1)
        compare(importWorkflow.openDraftCalls, 1)
        compare(importWorkflow.lastOpenedDraftId, "draft-1")
    }

    function test_draftStatusRunClickFallsBackToLogIdWhenDraftFlagIsMissing() {
        importWorkflow.runs = [
            { logId: "import-legacy", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", draftAttached: false, draftId: "", statementId: "" }
        ]

        var view = createView()
        var runMouse = findRequired(view, "runLogRow_import-legacy")

        verify(runMouse.enabled)
        runMouse.activateRun()

        compare(importWorkflow.activateCalls, 1)
        compare(importWorkflow.openDraftCalls, 1)
        compare(importWorkflow.lastOpenedDraftId, "import-legacy")
    }

    function test_finalizedRunClickNavigatesToBookingStatement() {
        importWorkflow.runs = [
            { logId: "import-2", time: "2026-05-16 10:00:00", status: "Finalized", file: "/tmp/import.pdf", message: "done", payload: "", draftAttached: false, draftId: "", statementId: "statement-2" }
        ]

        var view = createView()
        var runMouse = findRequired(view, "runLogRow_import-2")

        verify(runMouse.enabled)
        runMouse.activateRun()
        wait(0)

        compare(session.selectedStatementId, "statement-2")
        compare(session.selectedTransactionId, "")
        compare(navigation.sectionValue, 3)
        compare(navigation.bookingViewValue, 0)
    }

    function test_deleteButtonRemovesRun() {
        importWorkflow.runs = [
            { logId: "import-3", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", draftAttached: true, draftId: "draft-3", statementId: "" }
        ]

        var view = createView()
        var deleteButton = findRequired(view, "runLogDelete_import-3")

        deleteButton.clicked()

        compare(importWorkflow.clearDraftCalls, 1)
        compare(importWorkflow.removeCalls, 1)
    }

    function test_selectedDraftRunIsHighlighted() {
        importWorkflow.runs = [
            { logId: "import-2", time: "2026-05-16 10:00:00", status: "Draft", file: "/tmp/import.pdf", message: "draft", payload: "", draftAttached: true, draftId: "draft-2", statementId: "" }
        ]
        importWorkflow.draft = ({ draftId: "draft-2" })

        var view = createView()
        var selectedCard = findRequired(view, "runLogCard_import-2")

        compare(selectedCard.border.color, theme.selectionHighlight)
    }
}
