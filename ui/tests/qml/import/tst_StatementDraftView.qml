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

    property var importWorkflow: QtObject {
        property int persistCalls: 0
        property int clearPersistedCalls: 0
        property int finalizeCalls: 0
        property string finalizeResult: "statement-finalized"
        property string lastClearedDraftId: ""
        property int clearDraftCalls: 0
        property int runNoteCalls: 0
        property int amountUpdateCalls: 0
        property int createActorCalls: 0
        property int createPropertyCalls: 0
        property int createContractCalls: 0
        property string lastAmountText: ""
        property string lastActorName: ""
        property string lastPropertyName: ""
        property string lastContractName: ""
        property string lastContractType: ""
        property var lastRunNote: ({})

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
        function updateCurrentAmount(draft, text) {
            amountUpdateCalls += 1
            lastAmountText = text
            if (!draft || !draft.transactions || !draft.transactions.setAmount) return
            var value = Number(text)
            if (!isNaN(value)) draft.transactions.setAmount(draft.currentIndex, value)
        }
        function currentTransactionViewState(draft) { return ({ actorChoices: [], effectiveAllocatable: false }) }
        function createActorChoiceForCurrentDraft(draft, actorName) {
            createActorCalls += 1
            lastActorName = String(actorName || "")
            var name = String(actorName || "").trim()
            if (name.length === 0) return ({})
            var rows = testCase.session._actorRows.slice()
            rows.push({ id: "actor-new", name: name, display: name })
            testCase.session._actorRows = rows
            testCase.session.dataRevision += 1
            return ({ id: "actor-new", display: name, name: name })
        }
        function createPropertyChoiceForCurrentDraft(draft, propertyName) {
            createPropertyCalls += 1
            lastPropertyName = String(propertyName || "")
            var name = String(propertyName || "").trim()
            if (name.length === 0) return ({})
            return ({ id: "property-new", display: name, name: name })
        }
        function createOrSelectContractChoiceForCurrentDraft(draft, contractName, contractType, allocatableMode) {
            createContractCalls += 1
            lastContractName = String(contractName || "")
            lastContractType = String(contractType || "")
            var type = String(contractType || "").trim()
            if (type.length === 0) return ({})
            var name = String(contractName || "").trim()
            if (name.length === 0) name = "Contract 1"
            return ({ id: "contract-new", display: name, name: name, type: type, actorIds: [], propertyIds: [] })
        }
        function selectCurrentActorChoice(draft, row) {
            if (!draft || !draft.transactions || !row) return
            draft.transactions.setActorId(draft.currentIndex, row.id || "")
            draft.transactions.setActorText(draft.currentIndex, "")
            draft.transactions.setActorSelected(draft.currentIndex, true)
            if (draft.refresh) draft.refresh()
        }
        function selectCurrentContractChoice(draft, row) {
            if (!draft || !draft.transactions || !row) return
            draft.transactions.setContractId(draft.currentIndex, row.id || "")
            draft.transactions.setContractSelected(draft.currentIndex, true)
            if (draft.refresh) draft.refresh()
        }
        function clearDraft() { clearDraftCalls += 1 }
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

    property var session: QtObject {
        property int dataRevision: 0
        property var _actorRows: []
        property var actors: QtObject {
            function findRowById(id) { return -1 }
            function get(index) { return null }
        }
        property var contracts: QtObject {
            function findRowById(id) { return -1 }
            function get(index) { return null }
        }
        function actorRows() { return _actorRows || [] }
        function propertyRows() { return [] }
    }

    property var appContext: QtObject {
        property var importWorkflow: testCase.importWorkflow
        property var navigation: testCase.navigation
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int chartValueLabelWidth: 120
        property int viewCompactActionButtonSize: 28
        property int spacing: 8
        property int panelPadding: 12
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
        return Qt.createQmlObject('import QtQml 2.15; QtObject { id: draftRoot; signal changed(); property string draftId: "draft-1"; property string name: "Draft Name"; property int currentIndex: 0; property int count: 2; property var current: ({ id: "tx-1", name: "Tx 1", status: 0, actorText: "", type: "", bookingDate: "", valuta: "", amount: 12.5, metadata: "Kundennr: 12345", proofImageData: "/9j/4AAQSkZJRgABAQAAZABkAAD/2wCEABQQEBkSGScXFycyJh8mMi4mJiYmLj41NTU1NT5EQUFBQUFBREREREREREREREREREREREREREREREREREREREQBFRkZIBwgJhgYJjYmICY2RDYrKzZERERCNUJERERERERERERERERERERERERERERERERERERERERERERERERERP/AABEIAAEAAQMBIgACEQEDEQH/xABMAAEBAAAAAAAAAAAAAAAAAAAABQEBAQAAAAAAAAAAAAAAAAAABQYQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AJQA9Yv/2Q==", contractId: "", propertyIds: [] }); property var rows: [{ id: "tx-1", name: "Tx 1", status: 0, actorText: "", type: "", bookingDate: "", valuta: "", amount: 12.5, metadata: "Kundennr: 12345", proofImageData: "/9j/4AAQSkZJRgABAQAAZABkAAD/2wCEABQQEBkSGScXFycyJh8mMi4mJiYmLj41NTU1NT5EQUFBQUFBREREREREREREREREREREREREREREREREREREREQBFRkZIBwgJhgYJjYmICY2RDYrKzZERERCNUJERERERERERERERERERERERERERERERERERERERERERERERERERP/AABEIAAEAAQMBIgACEQEDEQH/xABMAAEBAAAAAAAAAAAAAAAAAAAABQEBAQAAAAAAAAAAAAAAAAAABQYQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AJQA9Yv/2Q==", contractId: "", propertyIds: [] }, { id: "tx-2", name: "Tx 2", status: 0, actorText: "", type: "", bookingDate: "", valuta: "", amount: 42.0, metadata: "", proofImageData: "", contractId: "", propertyIds: [] }]; property var transactions: QtObject { function updateField(index, key, value) { var copy = Object.assign({}, draftRoot.rows[index]); copy[key] = value; draftRoot.rows[index] = copy; if (index === draftRoot.currentIndex) draftRoot.current = copy; draftRoot.changed(); } function setStatus(index, value) { updateField(index, "status", value) } function setActorText(index, text) { updateField(index, "actorText", text) } function setActorId(index, id) { updateField(index, "actorId", id) } function setActorSelected(index, selected) {} function setNewActorSelected(index, selected) {} function setType(index, value) { updateField(index, "type", value) } function setContractId(index, id) { updateField(index, "contractId", id) } function setContractSelected(index, selected) {} function setNewContractSelected(index, selected) {} function setProperties(index, values) { updateField(index, "propertyIds", values || []) } function setBookingDate(index, value) { updateField(index, "bookingDate", value) } function setValuta(index, value) { updateField(index, "valuta", value) } function setName(index, value) { updateField(index, "name", value) } function setAmount(index, value) { updateField(index, "amount", value) } function setMetadata(index, value) { updateField(index, "metadata", value) } function setAllocatable(index, value) { updateField(index, "allocatable", value) } function setAllocatableSelected(index, value) {} function setAllocatableManualOverride(index, value) {} function removeTransaction(index) {} } function selectCurrent() { current = rows[currentIndex]; changed(); } function prev() { if (count > 0) { currentIndex = (currentIndex + count - 1) % count; selectCurrent(); } } function next() { if (count > 0) { currentIndex = (currentIndex + 1) % count; selectCurrent(); } } function insertTransactionAfterCurrent() { count += 1; currentIndex += 1 } function removeTransaction(index) { if (count > 1) count -= 1 } function refresh() { changed(); } }', testCase)
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
        importWorkflow.finalizeCalls = 0
        importWorkflow.persistCalls = 0
        importWorkflow.clearPersistedCalls = 0
        importWorkflow.finalizeResult = "statement-finalized"
        importWorkflow.lastClearedDraftId = ""
        importWorkflow.clearDraftCalls = 0
        importWorkflow.runNoteCalls = 0
        importWorkflow.amountUpdateCalls = 0
        importWorkflow.createActorCalls = 0
        importWorkflow.createPropertyCalls = 0
        importWorkflow.createContractCalls = 0
        importWorkflow.lastAmountText = ""
        importWorkflow.lastActorName = ""
        importWorkflow.lastPropertyName = ""
        importWorkflow.lastContractName = ""
        importWorkflow.lastContractType = ""
        importWorkflow.lastRunNote = ({})

        navigation.sectionValue = -1
        session.dataRevision = 0
        session._actorRows = []
        draftObject = createDraftObject()
    }

    function test_IMP_D_001_returnWritesDraftNoteAndNavigatesBack() {
        var view = createView()

        view.returnToImport()

        compare(importWorkflow.runNoteCalls, 1)
        compare(importWorkflow.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
    }

    function test_IMP_D_002_discardClearsPersistedDraftAndNavigatesBack() {
        var view = createView()

        view.discardDraft()

        compare(importWorkflow.clearPersistedCalls, 1)
        compare(importWorkflow.lastClearedDraftId, "draft-1")
        compare(importWorkflow.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
    }

    function test_IMP_D_003_finalizeSuccessClearsPersistedDraftAndLogsFinalized() {
        var view = createView()

        view.finalizeDraft()
        wait(0)

        compare(importWorkflow.finalizeCalls, 1)
        compare(importWorkflow.clearPersistedCalls, 1)
        compare(importWorkflow.clearDraftCalls, 1)
        compare(navigation.sectionValue, 4)
        verify(String(importWorkflow.lastRunNote.status).indexOf("Finalized") !== -1)
    }

    function test_IMP_D_004_finalizeFailureKeepsDraftAvailable() {
        importWorkflow.finalizeResult = ""
        var view = createView()

        view.finalizeDraft()

        compare(importWorkflow.finalizeCalls, 1)
        compare(importWorkflow.clearPersistedCalls, 0)
        compare(importWorkflow.clearDraftCalls, 0)
        verify(String(importWorkflow.lastRunNote.status).indexOf("Finalize failed") !== -1)
    }

    function test_IMP_D_006_deleteTransactionRemovesCurrentTransaction() {
        var view = createView()
        var deleteButton = findRequired(view, "statementDraftDeleteTransactionButton")

        deleteButton.clicked()

        compare(draftObject.count, 1)
        compare(importWorkflow.persistCalls, 1)
    }

    function test_IMP_D_009_addTransactionAddsCurrentTransaction() {
        var view = createView()
        var addButton = findRequired(view, "statementDraftAddTransactionButton")

        addButton.clicked()

        compare(draftObject.count, 3)
        compare(importWorkflow.persistCalls, 1)
    }

    function test_IMP_D_005_transactionNavigationUpdatesCurrentIndex() {
        var view = createView()
        draftObject.currentIndex = 0

        draftObject.next()
        compare(draftObject.currentIndex, 1)

        draftObject.prev()
        compare(draftObject.currentIndex, 0)
    }

    function test_IMP_D_007_transactionNavigationWrapsAtEdges() {
        var view = createView()
        draftObject.currentIndex = 1

        draftObject.next()
        compare(draftObject.currentIndex, 0)

        draftObject.prev()
        compare(draftObject.currentIndex, 1)
    }

    function test_IMP_D_008_metadataAndProofAreRenderedFromCurrentDraft() {
        var view = createView()
        var metadata = findRequired(view, "transactionDraftMetadataTextArea")
        var proof = findRequired(view, "transactionDraftProofImage")

        compare(metadata.text, "Kundennr: 12345")
        compare(proof.source.toString(), "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAZABkAAD/2wCEABQQEBkSGScXFycyJh8mMi4mJiYmLj41NTU1NT5EQUFBQUFBREREREREREREREREREREREREREREREREREREREQBFRkZIBwgJhgYJjYmICY2RDYrKzZERERCNUJERERERERERERERERERERERERERERERERERERERERERERERERERP/AABEIAAEAAQMBIgACEQEDEQH/xABMAAEBAAAAAAAAAAAAAAAAAAAABQEBAQAAAAAAAAAAAAAAAAAABQYQAQAAAAAAAAAAAAAAAAAAAAARAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AJQA9Yv/2Q==")
    }

    function test_IMP_D_010_amountCommitsOnEditingFinished() {
        var view = createView()
        var amount = findRequired(view, "transactionDraftAmountField")

        compare(amount.text, "12.5")

        amount.text = "99.99"
        wait(0)

        compare(amount.text, "99.99")
        compare(importWorkflow.amountUpdateCalls, 0)

        amount.editingFinished()
        wait(0)

        compare(importWorkflow.amountUpdateCalls, 1)
        compare(importWorkflow.lastAmountText, "99.99")
    }

    function test_IMP_D_011_amountStaysConsistentAcrossTransactionNavigation() {
        var view = createView()
        var amount = findRequired(view, "transactionDraftAmountField")

        compare(amount.text, "12.5")

        amount.text = "99.99"
        amount.editingFinished()
        wait(0)
        compare(amount.text, "99.99")

        draftObject.next()
        wait(0)
        compare(draftObject.currentIndex, 1)
        compare(amount.text, "42")

        draftObject.prev()
        wait(0)
        compare(draftObject.currentIndex, 0)
        compare(amount.text, "99.99")
    }

    function test_IMP_D_012_propertyAddUsesWorkflowMethod() {
        var view = createView()
        var field = findRequired(view, "transactionDraftPropertyNameInput")
        var addButton = findRequired(view, "transactionDraftPropertyAddButton")

        field.text = "New Property"
        field.textEdited()
        addButton.clicked()

        compare(importWorkflow.createPropertyCalls, 1)
        compare(importWorkflow.lastPropertyName, "New Property")
    }

    function test_IMP_D_013_contractAddUsesWorkflowMethod() {
        var view = createView()
        var addButton = findRequired(view, "transactionDraftContractAddButton")

        // direkte Draft-Vorbelegung für deterministisches Aktivieren
        draftObject.transactions.setType(draftObject.currentIndex, "Lease")
        wait(0)

        addButton.clicked()

        compare(importWorkflow.createContractCalls, 1)
        compare(importWorkflow.lastContractType, "Lease")
    }

    function test_IMP_D_014_actorAddAppearsImmediatelyInDropdownAndGetsSelected() {
        var view = createView()
        var actorInput = findRequired(view, "transactionDraftActorTextField")
        var actorAddButton = findRequired(view, "transactionDraftActorAddFromTextButton")
        var actorCombo = findRequired(view, "transactionDraftActorChoiceCombo")

        actorInput.text = "Alice Example"
        actorInput.textEdited()
        actorAddButton.clicked()
        wait(0)

        compare(importWorkflow.createActorCalls, 1)
        compare(importWorkflow.lastActorName, "Alice Example")
        verify(actorCombo.model.length >= 2)
        compare(String(actorCombo.model[1].id), "actor-new")
        compare(draftObject.current.actorId, "actor-new")
        verify(actorCombo.currentIndex > 0)
    }

    function test_IMP_D_015_contractTypeCommitsOnlyOnExplicitCommit() {
        var view = createView()
        var typeField = findRequired(view, "transactionDraftContractTypeField")

        compare(String(draftObject.current.type || ""), "")
        typeField.text = "Strom"
        typeField.textEdited()
        wait(0)
        compare(String(draftObject.current.type || ""), "")

        typeField.editingFinished()
        wait(0)
        compare(String(draftObject.current.type || ""), "Strom")
    }

    function test_IMP_D_016_commitPendingEditsPersistsContractAndActorFields() {
        var view = createView()
        var contractNameField = findRequired(view, "transactionDraftContractNameField")
        var contractTypeField = findRequired(view, "transactionDraftContractTypeField")
        var actorField = findRequired(view, "transactionDraftActorTextField")

        contractNameField.text = "My Contract"
        contractNameField.textEdited()
        contractTypeField.text = "Electricity"
        contractTypeField.textEdited()
        actorField.text = "Actor X"
        actorField.textEdited()

        view.commitPendingEdits()
        wait(0)

        compare(String(draftObject.current.type || ""), "Electricity")
        compare(String(draftObject.current.actorText || ""), "Actor X")
    }
}
