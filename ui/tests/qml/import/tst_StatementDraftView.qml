/**
 * @file ui/tests/qml/import/tst_StatementDraftView.qml
 * @brief Provides QML tests for StatementDraftView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "StatementDraftViewTests"
    when: windowShown
    width: 960
    height: 640

    property var importController: QtObject {
        property int clearDraftCalls: 0
        property int runNoteCalls: 0
        property var lastRunNote: ({})

        function clearDraft() { clearDraftCalls += 1 }
        function addRunNote(statusText, messageText, draftAttached, statementId) {
            runNoteCalls += 1
            lastRunNote = {
                status: statusText,
                message: messageText,
                draftAttached: draftAttached,
                statementId: statementId || ""
            }
        }
    }

    property var draftController: QtObject {
        property int persistCalls: 0
        property int clearPersistedCalls: 0
        property int finalizeCalls: 0
        property string finalizeResult: "statement-finalized"
        property string lastClearedDraftId: ""

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
    }

    property var navigation: QtObject {
        property int sectionValue: -1
        function setSectionValue(value) { sectionValue = value }
    }

    property var session: QtObject {
        property var actors: QtObject {
            function findRowById(id) { return -1 }
            function get(index) { return null }
        }
        property var contracts: QtObject {
            function findRowById(id) { return -1 }
            function get(index) { return null }
        }
        function propertyRows() { return [] }
    }

    property var appContext: QtObject {
        property var importController: testCase.importController
        property var draftController: testCase.draftController
        property var navigation: testCase.navigation
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int chartValueLabelWidth: 120
        property int viewCompactActionButtonSize: 28
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color successStrong: "#008800"
        property color warning: "#aa8800"
        property color danger: "#aa0000"
        property color success: "#008800"
        property int controlHeight: 32
    }

    Component {
        id: statementDraftViewComponent

        StatementDraftView {
            width: 900
            height: 600
            appContext: testCase.appContext
            theme: testCase.theme
            draft: testCase.draftObject
        }
    }

    property var draftObject: null

    function createDraftObject() {
        return Qt.createQmlObject('import QtQml 2.15; QtObject { property string draftId: "draft-1"; property string name: "Draft Name"; property int currentIndex: 0; property int count: 2; property var current: ({ id: "tx-1", name: "Tx 1", status: 0, actorText: "", type: "", bookingDate: "", valuta: "", amount: 12.5, contractId: "", propertyIds: [] }); property var transactions: QtObject { function setStatus(index, value) {} function setActorText(index, text) {} function setActorId(index, id) {} function setNewActorSelected(index, selected) {} function setType(index, value) {} function setContractId(index, id) {} function setNewContractSelected(index, selected) {} function setBookingDate(index, value) {} function setValuta(index, value) {} function setName(index, value) {} function setAllocatable(index, value) {} function setAllocatableManualOverride(index, value) {} function removeTransaction(index) {} } function prev() { if (currentIndex > 0) currentIndex -= 1 } function next() { if (currentIndex < count - 1) currentIndex += 1 } function removeTransaction(index) { if (count > 1) count -= 1 } function refresh() {} }', testCase)
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(statementDraftViewComponent, testCase)
    }

    function init() {
        importController.clearDraftCalls = 0
        importController.runNoteCalls = 0
        importController.lastRunNote = ({})

        draftController.persistCalls = 0
        draftController.clearPersistedCalls = 0
        draftController.finalizeCalls = 0
        draftController.finalizeResult = "statement-finalized"
        draftController.lastClearedDraftId = ""

        navigation.sectionValue = -1
        draftObject = createDraftObject()
    }

    function test_IMP_D_001_returnWritesDraftNoteAndNavigatesBack() {
        var view = createView()

        view.returnToImport()

        compare(importController.runNoteCalls, 1)
        compare(importController.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
    }

    function test_IMP_D_002_discardClearsPersistedDraftAndNavigatesBack() {
        var view = createView()

        view.discardDraft()

        compare(draftController.clearPersistedCalls, 1)
        compare(draftController.lastClearedDraftId, "draft-1")
        compare(importController.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
    }

    function test_IMP_D_003_finalizeSuccessClearsPersistedDraftAndLogsFinalized() {
        var view = createView()

        view.finalizeDraft()
        wait(0)

        compare(draftController.finalizeCalls, 1)
        compare(draftController.clearPersistedCalls, 1)
        compare(importController.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
        verify(String(importController.lastRunNote.status).indexOf("Finalized") !== -1)
    }

    function test_IMP_D_004_finalizeFailureKeepsDraftAvailable() {
        draftController.finalizeResult = ""
        var view = createView()

        view.finalizeDraft()

        compare(draftController.finalizeCalls, 1)
        compare(draftController.clearPersistedCalls, 0)
        compare(importController.clearDraftCalls, 0)
        verify(String(importController.lastRunNote.status).indexOf("Finalize failed") !== -1)
    }

    function test_IMP_D_006_deleteTransactionRemovesCurrentTransaction() {
        var view = createView()
        var deleteButton = findRequired(view, "statementDraftDeleteTransactionButton")

        deleteButton.clicked()

        compare(draftObject.count, 1)
    }

    function test_IMP_D_005_transactionNavigationUpdatesCurrentIndex() {
        var view = createView()
        draftObject.currentIndex = 0

        draftObject.next()
        compare(draftObject.currentIndex, 1)

        draftObject.prev()
        compare(draftObject.currentIndex, 0)
    }
}
