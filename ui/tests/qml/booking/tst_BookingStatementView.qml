/**
 * @file ui/tests/qml/booking/tst_BookingStatementView.qml
 * @brief Provides QML tests for BookingStatementView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingStatementViewTests"
    when: windowShown
    width: 960
    height: 640

    property var statementsById: ({})
    property var transactionsById: ({})
    property var statementTransactions: ({})
    property int dataRevision: 0

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

        function emptyTransactionDraft() {
            return {
                id: "",
                name: "",
                bookingDate: "",
                valuta: "",
                amount: 0.0,
                description: "",
                statementId: "",
                status: 0,
                actorId: "",
                allocatable: false,
                propertyIds: []
            }
        }

        function normalizeTransactionDraft(tx) {
            var source = tx || ({})
            var amountValue = Number(source.amount)
            if (isNaN(amountValue))
                amountValue = 0.0
            return {
                id: source.id || "",
                name: source.name || "",
                bookingDate: source.bookingDate || "",
                valuta: source.valuta || "",
                amount: amountValue,
                description: source.description || "",
                statementId: source.statementId || "",
                status: source.status !== undefined ? Number(source.status) : 0,
                actorId: source.actorId || "",
                contractId: source.contractId || "",
                allocatable: !!source.allocatable,
                propertyIds: source.propertyIds ? source.propertyIds.slice() : []
            }
        }

        function mapWithKeyValue(base, key, value) {
            var next = ({})
            var src = base || ({})
            for (var name in src)
                next[name] = src[name]
            next[key] = value
            return next
        }

        function contractRowById(rows, contractId) {
            var target = String(contractId || "")
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === target)
                    return list[i]
            }
            return null
        }

        function transactionDraft(draft, contractRows, changes) {
            var next = normalizeTransactionDraft(draft || ({ }))
            var update = changes || ({})

            if (update.contractId !== undefined) {
                var selectedContractId = String(update.contractId || "")
                var selectedRow = contractRowById(contractRows, selectedContractId)
                var selectedActorIds = selectedRow && selectedRow.actorIds ? selectedRow.actorIds : []
                var selectedPropertyIds = selectedRow && selectedRow.propertyIds ? selectedRow.propertyIds : []
                next = mapWithKeyValue(next, "contractId", selectedContractId)
                next = mapWithKeyValue(next, "actorId", selectedActorIds.length > 0 ? String(selectedActorIds[0] || "") : "")
                next = mapWithKeyValue(next, "propertyIds", selectedPropertyIds ? selectedPropertyIds.slice() : [])
            }

            if (update.actorId !== undefined) {
                var selectedActorId = String(update.actorId || "")
                var currentContractId = String(next.contractId || "")
                var actorRow = contractRowById(contractRows, currentContractId)
                var allowedActorIds = actorRow && actorRow.actorIds ? actorRow.actorIds : []
                var actorSupported = currentContractId.length === 0 || selectedActorId.length === 0
                if (!actorSupported) {
                    for (var i = 0; i < allowedActorIds.length; ++i) {
                        if (String(allowedActorIds[i] || "") === selectedActorId) {
                            actorSupported = true
                            break
                        }
                    }
                }
                next = mapWithKeyValue(next, "actorId", selectedActorId)
                next = mapWithKeyValue(next, "contractId", actorSupported ? currentContractId : "")
            }

            if (update.propertyIds !== undefined) {
                var selectedIds = update.propertyIds ? update.propertyIds.slice() : []
                var contractId = String(next.contractId || "")
                var propertyRow = contractRowById(contractRows, contractId)
                var allowedPropertyIds = propertyRow && propertyRow.propertyIds ? propertyRow.propertyIds : []
                var propertiesSupported = contractId.length === 0
                if (!propertiesSupported) {
                    propertiesSupported = true
                    for (var p = 0; p < selectedIds.length; ++p) {
                        if (allowedPropertyIds.indexOf(String(selectedIds[p] || "")) === -1) {
                            propertiesSupported = false
                            break
                        }
                    }
                }
                next = mapWithKeyValue(next, "propertyIds", selectedIds)
                next = mapWithKeyValue(next, "contractId", propertiesSupported ? contractId : "")
            }

            return next
        }

        function addUniqueTrimmed(values, value) {
            var out = values ? values.slice() : []
            var next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeString(values, value) {
            var out = values ? values.slice() : []
            var idx = out.indexOf(String(value || ""))
            if (idx !== -1)
                out.splice(idx, 1)
            return out
        }

        function indexOfId(rows, id) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return i
            }
            return -1
        }

        function indexOfString(values, id) {
            var list = values || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i]) === String(id || ""))
                    return i
            }
            return -1
        }

        function indexOfKeyValue(rows, key, value) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (Number(list[i][key]) === Number(value))
                    return i
            }
            return -1
        }

        function displayRowsWithEmpty(rows, emptyLabel, textKey) {
            var out = [{ id: "", display: emptyLabel }]
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                out.push({
                    id: String(list[i].id || ""),
                    display: String(list[i][textKey] || list[i].name || "")
                })
            }
            return out
        }

        function rowIds(rows) {
            var out = []
            var list = rows || []
            for (var i = 0; i < list.length; ++i)
                out.push(String(list[i].id || ""))
            return out
        }

        function wrappedIndex(index, total) {
            if (total <= 0)
                return 0
            return (index + total) % total
        }

        function currentDraftState(drafts, index, fallbackDraft) {
            var list = drafts && drafts.length > 0 ? drafts.slice() : [fallbackDraft]
            var currentIndex = Math.max(0, Math.min(index, list.length - 1))
            return { drafts: list, index: currentIndex, draft: list[currentIndex] }
        }

        function setCurrentDraft(drafts, index, draft, fallbackDraft) {
            var state = currentDraftState(drafts, index, fallbackDraft)
            state.drafts[state.index] = normalizeTransactionDraft(draft)
            return { drafts: state.drafts, index: state.index }
        }

        function createDraftListState(drafts, index, fallbackDraft) {
            return currentDraftState(drafts, index, fallbackDraft)
        }

        function insertDraftAfterCurrent(drafts, index, fallbackDraft) {
            var state = currentDraftState(drafts, index, fallbackDraft)
            var next = state.drafts.slice()
            var insertIndex = Math.max(0, Math.min(state.index + 1, next.length))
            next.splice(insertIndex, 0, normalizeTransactionDraft(fallbackDraft))
            return { drafts: next, index: insertIndex }
        }

        function removeDraftAt(drafts, index, fallbackDraft) {
            var state = currentDraftState(drafts, index, fallbackDraft)
            var next = state.drafts.slice()
            if (next.length > 1)
                next.splice(state.index, 1)
            if (next.length === 0)
                next = [normalizeTransactionDraft(fallbackDraft)]
            var nextIndex = Math.max(0, Math.min(state.index, next.length - 1))
            return { drafts: next, index: nextIndex }
        }

        function statementTransactionRows(statementId) {
            var ids = testCase.statementTransactions[String(statementId || "")] || []
            var out = []
            for (var i = 0; i < ids.length; ++i) {
                var tx = testCase.transactionsById[String(ids[i])] || ({})
                if (tx.id)
                    out.push({ id: String(tx.id), name: String(tx.name || "") })
            }
            return out
        }

        function orderedSelectionState(rows, preferred, index, selectedId, key) {
            var list = rows || []
            var preferredIds = preferred || []
            var ids = preferredIds.length > 0 ? preferredIds.slice() : rowIds(list)
            if (ids.length === 0)
                return { rows: list, orderIds: [], index: -1, id: "" }

            var currentIndex = -1
            if (selectedId && String(selectedId).length > 0)
                currentIndex = ids.indexOf(String(selectedId))
            if (currentIndex < 0)
                currentIndex = Math.max(0, Math.min(index, ids.length - 1))
            var currentId = String(ids[currentIndex])
            return { rows: list, orderIds: ids, index: currentIndex, id: currentId, currentId: currentId }
        }

        function navigateSelectionState(rows, index, selectedId, delta, fallbackIndex, key) {
            var state = orderedSelectionState(rows, rowIds(rows), index, selectedId, key)
            if (state.orderIds.length === 0)
                return { index: -1, id: "" }
            var nextIndex = wrappedIndex(state.index + delta, state.orderIds.length)
            return { index: nextIndex, id: String(state.orderIds[nextIndex]) }
        }

        function orderWithInsertedId(previousOrder, updatedIds, newId, insertAfterIndex) {
            var base = previousOrder ? previousOrder.slice() : []
            var updated = updatedIds || []
            var id = String(newId || "")
            var next = []
            for (var i = 0; i < base.length; ++i) {
                if (updated.indexOf(base[i]) !== -1)
                    next.push(base[i])
            }
            if (next.indexOf(id) === -1) {
                var at = Math.max(0, Math.min(insertAfterIndex + 1, next.length))
                next.splice(at, 0, id)
            }
            for (var j = 0; j < updated.length; ++j) {
                if (next.indexOf(updated[j]) === -1)
                    next.push(updated[j])
            }
            return next
        }

        function deleteReselectionState(updatedRows, previousOrder, previousIndex, deletedId, key) {
            var ids = rowIds(updatedRows)
            if (ids.length === 0)
                return { orderIds: [], index: -1, id: "" }
            var nextIndex = Math.max(0, Math.min(previousIndex, ids.length - 1))
            return { orderIds: ids, index: nextIndex, id: String(ids[nextIndex]) }
        }

        function navigatedId(rows, currentId, delta, fallbackIndex) {
            var list = rows || []
            if (list.length === 0)
                return ""
            var idx = indexOfId(list, currentId)
            if (idx < 0)
                idx = fallbackIndex
            else
                idx = wrappedIndex(idx + delta, list.length)
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
            if (parsed !== null)
                return parsed
            if (transactionId && testCase.transactionsById[String(transactionId || "")] && testCase.transactionsById[String(transactionId || "")].amount !== undefined)
                return Number(testCase.transactionsById[String(transactionId || "")].amount)
            return Number(fallbackAmount || 0.0)
        }
    }

    property var statementController: QtObject {
        property int addCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        property var lastUpdate: ({})
        property string lastDeleteId: ""

        function reset() {
            addCalls = 0
            updateCalls = 0
            deleteCalls = 0
            lastUpdate = ({})
            lastDeleteId = ""
        }

        function addStatement(name) {
            addCalls += 1
            var id = "statement-new"
            testCase.statementsById[id] = { id: id, name: name }
            testCase.session.statements = [{ id: id, name: name }]
            return id
        }

        function updateStatement(id, name) {
            updateCalls += 1
            lastUpdate = { id: id, name: name }
            if (testCase.statementsById[id])
                testCase.statementsById[id].name = name
        }

        function deleteStatement(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var transactionController: QtObject {
        property int addTransactionsCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        property int addTransactionCalls: 0
        property var lastUpdate: ({})

        function reset() {
            addTransactionsCalls = 0
            updateCalls = 0
            deleteCalls = 0
            addTransactionCalls = 0
            lastUpdate = ({})
        }

        function addTransactions(statementId, drafts) {
            addTransactionsCalls += 1
            var draftList = drafts || []
            var ids = []
            for (var i = 0; i < draftList.length; ++i) {
                var id = "tx-new-" + i
                var tx = testCase.session.normalizeTransactionDraft(draftList[i])
                tx.id = id
                tx.statementId = statementId
                testCase.transactionsById[id] = tx
                ids.push(id)
            }
            testCase.statementTransactions[statementId] = ids
            return ids
        }

        function addTransaction(name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
            addTransactionCalls += 1
            var id = "tx-added-" + addTransactionCalls
            testCase.transactionsById[id] = {
                id: id,
                name: name,
                bookingDate: bookingDate,
                valuta: valuta,
                amount: amount,
                description: "",
                statementId: statementId,
                status: status,
                actorId: actorId,
                contractId: contractId,
                allocatable: allocatable,
                propertyIds: propertyIds || []
            }
            var ids = testCase.statementTransactions[statementId] || []
            ids.push(id)
            testCase.statementTransactions[statementId] = ids
            return id
        }

        function insertTransactionAfter(afterTransactionId, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
            addTransactionCalls += 1
            var id = "tx-added-" + addTransactionCalls
            testCase.transactionsById[id] = {
                id: id,
                name: name,
                bookingDate: bookingDate,
                valuta: valuta,
                amount: amount,
                description: "",
                statementId: statementId,
                status: status,
                actorId: actorId,
                contractId: contractId,
                allocatable: allocatable,
                propertyIds: propertyIds || []
            }
            var ids = (testCase.statementTransactions[statementId] || []).slice()
            var at = ids.indexOf(String(afterTransactionId || ""))
            if (at < 0)
                ids.push(id)
            else
                ids.splice(at + 1, 0, id)
            testCase.statementTransactions[statementId] = ids
            return id
        }

        function transaction(id) {
            return testCase.transactionsById[String(id || "")] || ({})
        }

        function updateTransaction(id, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds) {
            updateCalls += 1
            lastUpdate = {
                id: id,
                name: name,
                bookingDate: bookingDate,
                valuta: valuta,
                amount: amount,
                statementId: statementId,
                status: status,
                actorId: actorId,
                contractId: contractId,
                allocatable: allocatable,
                propertyIds: propertyIds
            }
        }

        function deleteTransaction(id) {
            deleteCalls += 1
            var tx = testCase.transactionsById[String(id || "")] || ({})
            var statementId = String(tx.statementId || "")
            delete testCase.transactionsById[String(id || "")]
            var ids = testCase.statementTransactions[statementId] || []
            var next = []
            for (var i = 0; i < ids.length; ++i) {
                if (String(ids[i]) !== String(id || ""))
                    next.push(ids[i])
            }
            testCase.statementTransactions[statementId] = next
        }
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
                if (parsed !== null)
                    return parsed
                if (transactionId && testCase.transactionsById[String(transactionId || "")] && testCase.transactionsById[String(transactionId || "")].amount !== undefined)
                    return Number(testCase.transactionsById[String(transactionId || "")].amount)
                return Number(fallbackAmount || 0.0)
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
        id: bookingComponent
        BookingStatementView {
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
        return createTemporaryObject(bookingComponent, testCase)
    }

    function init() {
        statementsById = ({})
        transactionsById = ({})
        statementTransactions = ({})
        statementController.reset()
        transactionController.reset()
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
        session.selectedStatement = null
        session.statements = []
        session.actors = [{ id: "actor-1", display: "Alice" }]
        session.contracts = [{ id: "contract-1", display: "Lease" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
    }

    function prepareEditStatement() {
        statementsById["statement-1"] = { id: "statement-1", name: "S1" }
        transactionsById["tx-1"] = {
            id: "tx-1",
            name: "Rent",
            bookingDate: "2026-01-01",
            valuta: "2026-01-01",
            amount: 10,
            description: "",
            statementId: "statement-1",
            status: 0,
            actorId: "",
            allocatable: false,
            propertyIds: []
        }
        statementTransactions["statement-1"] = ["tx-1"]
        session.statements = [{ id: "statement-1", name: "S1" }]
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.selectedTransactionId = "tx-1"
    }

    function test_createStatementCallsControllers() {
        var view = createView()
        view.createStatementName = "January"
        view.createStatementWithTransactions()

        compare(statementController.addCalls, 1)
        compare(transactionController.addTransactionCalls, 1)
        compare(session.selectedStatementId, "statement-new")
    }

    function test_createModeTransactionAddRemoveAndNavigation() {
        var view = createView()

        compare(view.createTransactions.length, 1)
        view.addTransactionAfterCurrent()
        compare(view.createTransactions.length, 2)

        view.navigateTransaction(1)
        compare(view.createTransactionIndex, 0)

        view.navigateTransaction(-1)
        compare(view.createTransactionIndex, 1)

        view.deleteCurrentTransaction()
        compare(view.createTransactions.length, 1)
    }

    function test_editModeTransactionAddInsertsAfterCurrentAndCarriesDates() {
        statementsById["statement-1"] = { id: "statement-1", name: "S1" }
        transactionsById["tx-1"] = {
            id: "tx-1",
            name: "Rent 1",
            bookingDate: "2026-01-01",
            valuta: "2026-01-03",
            amount: 10,
            description: "",
            statementId: "statement-1",
            status: 0,
            actorId: "",
            allocatable: false,
            propertyIds: []
        }
        transactionsById["tx-2"] = {
            id: "tx-2",
            name: "Rent 2",
            bookingDate: "2026-01-02",
            valuta: "2026-01-04",
            amount: 12,
            description: "",
            statementId: "statement-1",
            status: 0,
            actorId: "",
            allocatable: false,
            propertyIds: []
        }
        statementTransactions["statement-1"] = ["tx-1", "tx-2"]
        session.statements = [{ id: "statement-1", name: "S1" }]
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.selectedTransactionId = "tx-1"

        var view = createView()
        findRequired(view, "bookingStatementAddTransactionButton").clicked()
        wait(0)

        compare(statementTransactions["statement-1"].join(","), "tx-1,tx-added-1,tx-2")
        compare(session.selectedTransactionId, "tx-added-1")
        compare(transactionsById["tx-added-1"].bookingDate, "2026-01-01")
        compare(transactionsById["tx-added-1"].valuta, "2026-01-03")
    }

    function test_editModeUpdateAndDeleteStatement() {
        prepareEditStatement()
        var view = createView()
        var deleteButton = findRequired(view, "bookingDeleteButton")

        view.editStatementName = "S1-updated"
        view.updateSelectedEntity()
        compare(statementController.updateCalls, 1)
        compare(statementController.lastUpdate.id, "statement-1")
        compare(statementController.lastUpdate.name, "S1-updated")

        deleteButton.clicked()
        compare(statementController.deleteCalls, 1)
        compare(statementController.lastDeleteId, "statement-1")
    }

    function test_editModeUpdatePersistsTransactionAmountAndRelations() {
        prepareEditStatement()
        session.actors = [{ id: "actor-1", display: "Alice" }, { id: "actor-2", display: "Bob" }]
        session.properties = [{ id: "property-1", name: "Lot 1" }, { id: "property-2", name: "Lot 2" }]
        session.contracts = [{
            id: "contract-2",
            display: "Lease 2",
            actorIds: ["actor-2"],
            propertyIds: ["property-2"]
        }]

        var view = createView()
        var tx = session.mapWithKeyValue(view.editTransactionData, "amount", "12,75")
        tx = session.mapWithKeyValue(tx, "actorId", "actor-2")
        tx = session.mapWithKeyValue(tx, "contractId", "contract-2")
        tx = session.mapWithKeyValue(tx, "propertyIds", ["property-2"])
        view.editTransactionData = tx

        view.updateSelectedEntity()

        compare(transactionController.updateCalls, 1)
        compare(transactionController.lastUpdate.id, "tx-1")
        compare(transactionController.lastUpdate.actorId, "actor-2")
        compare(transactionController.lastUpdate.contractId, "contract-2")
        compare(transactionController.lastUpdate.propertyIds.length, 1)
        compare(transactionController.lastUpdate.propertyIds[0], "property-2")
        compare(transactionController.lastUpdate.amount, 12.75)
    }

    function test_transactionFieldsAndSelectorsUpdateEditedTransaction() {
        var view = createView()
        var base = view.createTransactions[0]
        var next = session.mapWithKeyValue(base, "name", "Tx Name")
        next = session.mapWithKeyValue(next, "actorId", "actor-1")
        next = session.mapWithKeyValue(next, "contractId", "contract-1")
        next = session.mapWithKeyValue(next, "propertyIds", ["property-1"])
        next = session.mapWithKeyValue(next, "allocatable", true)
        view.setCurrentCreateTransaction(next)

        compare(view.createTransactions[0].name, "Tx Name")
        compare(view.createTransactions[0].actorId, "actor-1")
        compare(view.createTransactions[0].contractId, "contract-1")
        compare(view.createTransactions[0].propertyIds.length, 1)
        compare(view.createTransactions[0].propertyIds[0], "property-1")
        compare(view.createTransactions[0].allocatable, true)
    }

    function test_amountFieldKeepsIntermediateTextInsteadOfNormalizingOnEveryKey() {
        var view = createView()
        var amountField = findRequired(view, "bookingTransactionAmountField")

        amountField.text = "-"
        amountField.textEdited()

        compare(view.createTransactions[0].amount, "-")
        compare(amountField.text, "-")

        amountField.text = "1,"
        amountField.textEdited()

        compare(view.createTransactions[0].amount, "1,")
        compare(amountField.text, "1,")
    }

    function test_contractSelectionAppliesActorAndPropertyFromSelectedContract() {
        session.actors = [
            { id: "actor-1", display: "Alice" },
            { id: "actor-2", display: "Bob" }
        ]
        session.properties = [
            { id: "property-1", name: "Lot 1" },
            { id: "property-2", name: "Lot 2" }
        ]
        session.contracts = [
            {
                id: "contract-1",
                display: "Lease",
                actorIds: ["actor-1"],
                propertyIds: ["property-1"]
            },
            {
                id: "contract-2",
                display: "Lease Plus",
                actorIds: ["actor-2"],
                propertyIds: ["property-2"]
            }
        ]

        var view = createView()
        var contractCombo = findRequired(view, "bookingTransactionContractComboBox")
        contractCombo.activated(2)

        compare(view.createTransactions[0].contractId, "contract-2")
        compare(view.createTransactions[0].actorId, "actor-2")
        compare(view.createTransactions[0].propertyIds.length, 1)
        compare(view.createTransactions[0].propertyIds[0], "property-2")
    }

    function test_actorSelectionClearsIncompatibleContractAndPersistsUpdate() {
        prepareEditStatement()
        session.actors = [
            { id: "actor-1", display: "Alice" },
            { id: "actor-2", display: "Bob" }
        ]
        session.contracts = [{
            id: "contract-1",
            display: "Lease",
            actorIds: ["actor-1"],
            propertyIds: []
        }]
        transactionsById["tx-1"].contractId = "contract-1"
        transactionsById["tx-1"].actorId = "actor-1"

        var view = createView()
        var actorCombo = findRequired(view, "bookingTransactionActorComboBox")
        actorCombo.activated(2)

        compare(view.editTransactionData.actorId, "actor-2")
        compare(view.editTransactionData.contractId, "")

        view.updateSelectedEntity()
        compare(transactionController.lastUpdate.actorId, "actor-2")
        compare(transactionController.lastUpdate.contractId, "")
    }

    function test_propertySelectionClearsIncompatibleContractAndPersistsUpdate() {
        prepareEditStatement()
        session.properties = [
            { id: "property-1", name: "Lot 1" },
            { id: "property-2", name: "Lot 2" }
        ]
        session.contracts = [{
            id: "contract-1",
            display: "Lease",
            actorIds: [],
            propertyIds: ["property-1"]
        }]
        transactionsById["tx-1"].contractId = "contract-1"
        transactionsById["tx-1"].propertyIds = ["property-1"]

        var view = createView()
        view.editTransactionData = session.mapWithKeyValue(view.editTransactionData, "propertyIds", ["property-2"])

        compare(view.editTransactionData.contractId, "contract-1")
        compare(view.editTransactionData.propertyIds.length, 1)
        compare(view.editTransactionData.propertyIds[0], "property-2")

        // Apply transaction-view policy path to mimic checkbox toggle semantics.
        var txView = findRequired(view, "bookingTransactionViewRoot")
        txView.applyPropertySelection(["property-2"])

        compare(view.editTransactionData.contractId, "")

        view.updateSelectedEntity()
        compare(transactionController.lastUpdate.contractId, "")
        compare(transactionController.lastUpdate.propertyIds.length, 1)
        compare(transactionController.lastUpdate.propertyIds[0], "property-2")
    }

    function test_editModeFieldEditsPersistViaUpdateButton() {
        prepareEditStatement()
        var view = createView()
        var nameField = findRequired(view, "bookingTransactionNameField")
        var bookingDateField = findRequired(view, "bookingTransactionBookingDateField")
        var valutaField = findRequired(view, "bookingTransactionValutaField")
        var amountField = findRequired(view, "bookingTransactionAmountField")

        nameField.text = "Rent Updated"
        nameField.textEdited()
        bookingDateField.text = "2026-02-01"
        bookingDateField.textEdited()
        valutaField.text = "2026-02-03"
        valutaField.textEdited()
        amountField.text = "99,50"
        amountField.textEdited()

        findRequired(view, "bookingUpdateButton").clicked()

        compare(transactionController.lastUpdate.name, "Rent Updated")
        compare(transactionController.lastUpdate.bookingDate, "2026-02-01")
        compare(transactionController.lastUpdate.valuta, "2026-02-03")
        compare(transactionController.lastUpdate.amount, 99.5)
    }

    function test_editModeAmountUpdateAcceptsDotDecimalAndInteger() {
        prepareEditStatement()
        var view = createView()

        view.editTransactionData = session.mapWithKeyValue(view.editTransactionData, "amount", "99.5")
        view.updateSelectedEntity()
        compare(transactionController.lastUpdate.amount, 99.5)

        view.editTransactionData = session.mapWithKeyValue(view.editTransactionData, "amount", "100")
        view.updateSelectedEntity()
        compare(transactionController.lastUpdate.amount, 100)
    }

    function test_statementNavigationChangesSelection() {
        session.statements = [
            { id: "statement-1", name: "S1" },
            { id: "statement-2", name: "S2" }
        ]
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }

        var view = createView()
        var nextStatement = findRequired(view, "bookingNextStatementButton")
        var prevStatement = findRequired(view, "bookingPreviousStatementButton")

        nextStatement.clicked()
        compare(session.selectedStatementId, "statement-2")

        nextStatement.clicked()
        compare(session.selectedStatementId, "")

        nextStatement.clicked()
        compare(session.selectedStatementId, "statement-1")

        prevStatement.clicked()
        compare(session.selectedStatementId, "")

        prevStatement.clicked()
        compare(session.selectedStatementId, "statement-2")
    }

    function test_statementNavigationStaysEnabledWithSingleRow() {
        session.statements = [
            { id: "statement-1", name: "S1" }
        ]

        var view = createView()
        var nextStatement = findRequired(view, "bookingNextStatementButton")
        var prevStatement = findRequired(view, "bookingPreviousStatementButton")

        compare(nextStatement.enabled, true)
        compare(prevStatement.enabled, true)

        nextStatement.clicked()
        compare(session.selectedStatementId, "statement-1")
    }

    function test_workspaceRevisionRebindsStatementAndTransactionLists() {
        var view = createView()
        var prevStatement = findRequired(view, "bookingPreviousStatementButton")
        var prevTransaction = findRequired(view, "bookingPreviousTransactionButton")

        compare(prevStatement.enabled, false)
        compare(prevTransaction.enabled, false)

        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.statements = [{ id: "statement-1", name: "S1" }]
        statementTransactions["statement-1"] = ["tx-1", "tx-2"]
        transactionsById["tx-1"] = { id: "tx-1", name: "Tx 1", statementId: "statement-1" }
        transactionsById["tx-2"] = { id: "tx-2", name: "Tx 2", statementId: "statement-1" }
        session.dataRevision += 1

        tryCompare(prevStatement, "enabled", true)
        tryCompare(prevTransaction, "enabled", true)
    }

    function test_selectedTransactionUpdatesInfoTextByRowPosition() {
        statementsById["statement-1"] = { id: "statement-1", name: "S1" }
        transactionsById["tx-1"] = {
            id: "tx-1",
            name: "Rent 1",
            bookingDate: "2026-01-01",
            valuta: "2026-01-01",
            amount: 10,
            description: "",
            statementId: "statement-1",
            status: 0,
            actorId: "",
            allocatable: false,
            propertyIds: []
        }
        transactionsById["tx-2"] = {
            id: "tx-2",
            name: "Rent 2",
            bookingDate: "2026-01-02",
            valuta: "2026-01-02",
            amount: 12,
            description: "",
            statementId: "statement-1",
            status: 0,
            actorId: "",
            allocatable: false,
            propertyIds: []
        }
        statementTransactions["statement-1"] = ["tx-1", "tx-2"]
        session.statements = [{ id: "statement-1", name: "S1" }]
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.selectedTransactionId = "tx-2"

        var view = createView()
        var infoLabel = findRequired(view, "bookingTransactionInfoLabel")

        compare(infoLabel.text, "Transaction 2 / 2")
    }

    function test_statementNavigationRemembersLastTransactionPerStatement() {
        statementsById["statement-1"] = { id: "statement-1", name: "S1" }
        statementsById["statement-2"] = { id: "statement-2", name: "S2" }
        transactionsById["tx-1a"] = { id: "tx-1a", name: "S1 Tx 1", statementId: "statement-1" }
        transactionsById["tx-1b"] = { id: "tx-1b", name: "S1 Tx 2", statementId: "statement-1" }
        transactionsById["tx-2a"] = { id: "tx-2a", name: "S2 Tx 1", statementId: "statement-2" }
        transactionsById["tx-2b"] = { id: "tx-2b", name: "S2 Tx 2", statementId: "statement-2" }
        statementTransactions["statement-1"] = ["tx-1a", "tx-1b"]
        statementTransactions["statement-2"] = ["tx-2a", "tx-2b"]
        session.statements = [
            { id: "statement-1", name: "S1" },
            { id: "statement-2", name: "S2" }
        ]
        session.selectedStatementId = "statement-1"
        session.selectedStatement = { id: "statement-1", name: "S1" }
        session.selectedTransactionId = "tx-1b"

        var view = createView()
        findRequired(view, "bookingNextStatementButton").clicked()
        compare(session.selectedStatementId, "statement-2")
        compare(session.selectedTransactionId, "tx-2a")

        session.selectedTransactionId = "tx-2b"
        findRequired(view, "bookingPreviousStatementButton").clicked()
        compare(session.selectedStatementId, "statement-1")
        compare(session.selectedTransactionId, "tx-1b")

        findRequired(view, "bookingNextStatementButton").clicked()
        compare(session.selectedStatementId, "statement-2")
        compare(session.selectedTransactionId, "tx-2b")
    }
}
