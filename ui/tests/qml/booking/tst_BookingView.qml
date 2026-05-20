/**
 * @file ui/tests/qml/booking/tst_BookingView.qml
 * @brief Provides QML tests for BookingView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
        property int dataRevision: 0
        property var selectedStatement: null
        property var statements: []
        property var actors: []
        property var contracts: []
        property var properties: []

        function statementRows() { return statements || [] }
        function actorRows() { return actors || [] }
        function contractRows() { return contracts || [] }
        function propertyRows() { return properties || [] }
        function emptyTransactionDraft() { return ({ id: "", name: "", bookingDate: "", valuta: "", amount: 0.0, description: "", statementId: "", status: 0, actorId: "", contractId: "", allocatable: false, propertyIds: [] }) }
        function normalizeTransactionDraft(tx) { return tx || emptyTransactionDraft() }
        function mapWithKeyValue(base, key, value) { var next = {}; var src = base || {}; for (var n in src) next[n] = src[n]; next[key] = value; return next }
        function contractRowById(rows, contractId) { var target = String(contractId || ""); var list = rows || []; for (var i = 0; i < list.length; ++i) { if (String(list[i].id || "") === target) return list[i] } return null }
        function transactionDraft(draft, contractRows, changes) { var next = normalizeTransactionDraft(draft || ({})); var update = changes || ({}); if (update.contractId !== undefined) { var selectedContractId = String(update.contractId || ""); var selectedRow = contractRowById(contractRows, selectedContractId); var selectedActorIds = selectedRow && selectedRow.actorIds ? selectedRow.actorIds : []; var selectedPropertyIds = selectedRow && selectedRow.propertyIds ? selectedRow.propertyIds : []; next = mapWithKeyValue(next, "contractId", selectedContractId); next = mapWithKeyValue(next, "actorId", selectedActorIds.length > 0 ? String(selectedActorIds[0] || "") : ""); next = mapWithKeyValue(next, "propertyIds", selectedPropertyIds ? selectedPropertyIds.slice() : []); } if (update.actorId !== undefined) { var selectedActorId = String(update.actorId || ""); var currentContractId = String(next.contractId || ""); var actorRow = contractRowById(contractRows, currentContractId); var allowedActorIds = actorRow && actorRow.actorIds ? actorRow.actorIds : []; var actorSupported = currentContractId.length === 0 || selectedActorId.length === 0; if (!actorSupported) { for (var i = 0; i < allowedActorIds.length; ++i) { if (String(allowedActorIds[i] || "") === selectedActorId) { actorSupported = true; break } } } next = mapWithKeyValue(next, "actorId", selectedActorId); next = mapWithKeyValue(next, "contractId", actorSupported ? currentContractId : ""); } if (update.propertyIds !== undefined) { var selectedIds = update.propertyIds ? update.propertyIds.slice() : []; var contractId = String(next.contractId || ""); var propertyRow = contractRowById(contractRows, contractId); var allowedPropertyIds = propertyRow && propertyRow.propertyIds ? propertyRow.propertyIds : []; var propertiesSupported = contractId.length === 0; if (!propertiesSupported) { propertiesSupported = true; for (var p = 0; p < selectedIds.length; ++p) { if (allowedPropertyIds.indexOf(String(selectedIds[p] || "")) === -1) { propertiesSupported = false; break } } } next = mapWithKeyValue(next, "propertyIds", selectedIds); next = mapWithKeyValue(next, "contractId", propertiesSupported ? contractId : ""); } return next }
        function addUniqueTrimmed(values, value) { var next = values ? values.slice() : []; var v = String(value || "").trim(); if (v.length === 0 || next.indexOf(v) !== -1) return next; next.push(v); return next }
        function removeString(values, value) { var next = values ? values.slice() : []; var i = next.indexOf(String(value || "")); if (i >= 0) next.splice(i, 1); return next }
        function indexOfId(rows, id) { var list = rows || []; for (var i = 0; i < list.length; ++i) { if (String(list[i].id || "") === String(id || "")) return i } return -1 }
        function indexOfString(values, id) { var list = values || []; for (var i = 0; i < list.length; ++i) { if (String(list[i]) === String(id || "")) return i } return -1 }
        function indexOfKeyValue(rows, key, value) { var list = rows || []; for (var i = 0; i < list.length; ++i) { if (Number(list[i][key]) === Number(value)) return i } return -1 }
        function displayRowsWithEmpty(rows, emptyLabel, textKey) { var out = [{ id: "", display: emptyLabel }]; var list = rows || []; for (var i = 0; i < list.length; ++i) out.push({ id: String(list[i].id || ""), display: String(list[i][textKey] || list[i].name || "") }); return out }
        function rowIds(rows) { var out = []; var list = rows || []; for (var i = 0; i < list.length; ++i) out.push(String(list[i].id || "")); return out }
        function wrappedIndex(index, total) { if (total <= 0) return 0; return (index + total) % total }
        function currentDraftState(drafts, index, fallbackDraft) { var list = drafts && drafts.length > 0 ? drafts.slice() : [fallbackDraft]; var currentIndex = Math.max(0, Math.min(index, list.length - 1)); return { drafts: list, index: currentIndex, draft: list[currentIndex] } }
        function setCurrentDraft(drafts, index, draft, fallbackDraft) { var state = currentDraftState(drafts, index, fallbackDraft); state.drafts[state.index] = normalizeTransactionDraft(draft); return { drafts: state.drafts, index: state.index } }
        function createDraftListState(drafts, index, fallbackDraft) { return currentDraftState(drafts, index, fallbackDraft) }
        function insertDraftAfterCurrent(drafts, index, fallbackDraft) { var state = currentDraftState(drafts, index, fallbackDraft); var next = state.drafts.slice(); var insertIndex = Math.max(0, Math.min(state.index + 1, next.length)); next.splice(insertIndex, 0, normalizeTransactionDraft(fallbackDraft)); return { drafts: next, index: insertIndex } }
        function removeDraftAt(drafts, index, fallbackDraft) { var state = currentDraftState(drafts, index, fallbackDraft); var next = state.drafts.slice(); if (next.length > 1) next.splice(state.index, 1); if (next.length === 0) next = [normalizeTransactionDraft(fallbackDraft)]; return { drafts: next, index: Math.max(0, Math.min(state.index, next.length - 1)) } }
        function statementTransactionRows(statementId) { return [] }
        function orderedSelectionState(rows, preferred, index, selectedId, key) { return { rows: rows || [], orderIds: [], index: -1, id: "" } }
        function navigateSelectionState(rows, index, selectedId, delta, fallbackIndex, key) { return { index: -1, id: "" } }
        function orderWithInsertedId(previousOrder, updatedIds, newId, insertAfterIndex) { return updatedIds || [] }
        function deleteReselectionState(updatedRows, previousOrder, previousIndex, deletedId, key) { return { orderIds: [], index: -1, id: "" } }
        function navigatedId(rows, currentId, delta, fallbackIndex) { var list = rows || []; if (list.length === 0) return ""; var idx = indexOfId(list, currentId); if (idx < 0) idx = fallbackIndex; else idx = wrappedIndex(idx + delta, list.length); return String(list[idx].id || "") }
        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) { var list = rows || []; var kept = []; for (var i = 0; i < list.length; ++i) { var rowId = String(list[i][key] || ""); if (rowId !== String(removedId || "")) kept.push(list[i]) } if (kept.length === 0) return ""; var idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1)); return String(kept[idx][key] || "") }
        function parseAmountString(value) { var text = String(value || "").trim(); if (text.length === 0) return null; text = text.replace(/\u00A0/g, "").replace(/\u202F/g, "").replace(/\s+/g, ""); if (text.length === 0) return null; var direct = Number(text); if (!isNaN(direct)) return direct; var decimalPos = -1; for (var i = text.length - 1; i >= 0; --i) { var ch = text.charAt(i); if (ch === "." || ch === ",") { decimalPos = i; break } } var canonical = ""; for (var j = 0; j < text.length; ++j) { var c = text.charAt(j); if (c >= "0" && c <= "9") { canonical += c; continue } if ((c === "-" || c === "+") && j === 0) { canonical += c; continue } if (c === "." || c === ",") { if (j === decimalPos) canonical += "."; continue } return null } if (canonical.length === 0 || canonical === "-" || canonical === "+") return null; var parsed = Number(canonical); return isNaN(parsed) ? null : parsed }
        function amountForTransactionCommit(rawAmount, transactionId, fallbackAmount) { if (typeof rawAmount === "number") return rawAmount; var parsed = parseAmountString(rawAmount); return parsed !== null ? parsed : Number(fallbackAmount || 0.0) }
    }

    property var statementController: QtObject {
        function addStatement(name) { return "statement-new" }
        function updateStatement(id, name) {}
        function deleteStatement(id) {}
    }

    property var transactionController: QtObject {
        function addTransactions(statementId, drafts) { return [] }
        function addTransaction(name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) { return "tx-new" }
        function insertTransactionAfter(afterTransactionId, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) { return "tx-new" }
        function transaction(id) { return ({}) }
        function updateTransaction(id, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {}
        function deleteTransaction(id) {}
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var sessionState: testCase.session
        property var workspaceFacade: QtObject {
            function parseAmountString(value) {
                var text = String(value || "").trim()
                if (text.length === 0)
                    return null
                text = text.replace(/\u00A0/g, "").replace(/\u202F/g, "").replace(/\s+/g, "")
                if (text.length === 0)
                    return null
                var direct = Number(text)
                if (!isNaN(direct))
                    return direct

                var decimalPos = -1
                for (var i = text.length - 1; i >= 0; --i) {
                    var ch = text.charAt(i)
                    if (ch === "." || ch === ",") {
                        decimalPos = i
                        break
                    }
                }

                var canonical = ""
                for (var j = 0; j < text.length; ++j) {
                    var c = text.charAt(j)
                    if (c >= "0" && c <= "9") {
                        canonical += c
                        continue
                    }
                    if ((c === "-" || c === "+") && j === 0) {
                        canonical += c
                        continue
                    }
                    if (c === "." || c === ",") {
                        if (j === decimalPos)
                            canonical += "."
                        continue
                    }
                    return null
                }
                if (canonical.length === 0 || canonical === "-" || canonical === "+")
                    return null
                var parsed = Number(canonical)
                return isNaN(parsed) ? null : parsed
            }
            function amountForTransactionCommit(rawAmount, transactionId, fallbackAmount) {
                if (typeof rawAmount === "number")
                    return rawAmount
                var parsed = parseAmountString(rawAmount)
                return parsed !== null ? parsed : Number(fallbackAmount || 0.0)
            }
            function addStatement(name) { return testCase.statementController.addStatement(name) }
            function updateStatement(id, name) { testCase.statementController.updateStatement(id, name) }
            function deleteStatement(id) { testCase.statementController.deleteStatement(id) }
            function addTransaction(name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
                return testCase.transactionController.addTransaction(name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds)
            }
            function insertTransactionAfter(afterTransactionId, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
                return testCase.transactionController.insertTransactionAfter(afterTransactionId, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds)
            }
            function transaction(id) { return testCase.transactionController.transaction(id) }
            function updateTransaction(id, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
                testCase.transactionController.updateTransaction(id, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds)
            }
            function deleteTransaction(id) { testCase.transactionController.deleteTransaction(id) }
        }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacingSmall: 6
        property int spacing: 8
        property int spacingLarge: 20
        property int panelPadding: 8
        property int viewCompactActionButtonSize: 28
        property int viewActionButtonWidth: 120
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int controlHeight: 32
        property int formFieldWidth: 220
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color textMuted: "#666666"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property string fontFamily: "Segoe UI"
        property int fontSize: 10
    }

    Component {
        id: bookingViewComponent
        BookingView {
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
        return createTemporaryObject(bookingViewComponent, testCase)
    }

    function init() {
        session.selectedStatementId = ""
        session.selectedStatement = null
        session.statements = []
        session.actors = [{ id: "actor-1", display: "Alice" }]
        session.contracts = [{ id: "contract-1", display: "Lease" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
    }

    function test_mountsStatementViewCreateMode() {
        var view = createView()
        var statementPanel = findRequired(view, "bookingStatementNameField")
        verify(statementPanel !== null)
    }

    function test_mountsStatementViewEditMode() {
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.statements = [{ id: "statement-1", name: "S1" }]

        var view = createView()
        var deleteButton = findRequired(view, "bookingDeleteButton")
        verify(deleteButton !== null)
    }
}
