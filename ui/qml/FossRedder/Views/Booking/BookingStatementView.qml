/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementView.qml
 * @brief Provides the BookingStatementView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var sessionState: root.appContext ? root.appContext.sessionState : null
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0

    property string createStatementName: ""
    property var createTransactions: [root.emptyTransaction()]
    property int createTransactionIndex: 0

    property string editStatementName: ""
    property var editTransactionData: root.emptyTransaction()
    property int editTransactionIndex: -1
    property var editTransactionOrderIds: []
    property string savedEditStatementName: ""
    property string savedEditTransactionJson: "{}"
    property var currentTransactionDraft: (root.emptyTransaction())
    property var lastTransactionIdByStatementId: ({})

    readonly property bool isCreateMode: !root.session || !root.session.selectedStatementId || root.session.selectedStatementId.length === 0

    function emptyTransaction() {
        return root.sessionState ? root.sessionState.emptyTransactionDraft() : ({})
    }

    function cloneTransaction(tx) {
        return root.sessionState ? root.sessionState.normalizeTransactionDraft(tx || ({})) : ({})
    }

    function amountForCommit(rawAmount, txId, fallbackValue) {
        if (!root.sessionState)
            return Number(fallbackValue || 0.0)
        return root.sessionState.amountForTransactionCommit(rawAmount, txId || "", Number(fallbackValue || 0.0))
    }

    function transactionSnapshot(data) {
        return JSON.stringify(root.cloneTransaction(data || root.emptyTransaction()))
    }

    function captureEditState() {
        root.savedEditStatementName = String(root.editStatementName || "")
        root.savedEditTransactionJson = root.transactionSnapshot(root.editTransactionData)
    }

    function hasEditChanges() {
        if (root.isCreateMode)
            return root.createStatementName.length > 0
        return root.savedEditStatementName !== String(root.editStatementName || "")
                || root.savedEditTransactionJson !== root.transactionSnapshot(root.editTransactionData)
    }

    function statementRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.statementRows() : []
    }

    function actorRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.actorRows() : []
    }

    function contractRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.contractRows() : []
    }

    function propertyRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.propertyRows() : []
    }

    function statementTransactionRows(statementId) {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.statementTransactionRows(statementId) : []
    }

    function editTransactionState() {
        const _workspaceRevision = root.workspaceRevision
        if (!root.session || !root.session.selectedStatementId)
            return ({ rows: [], orderIds: [], index: -1, id: "" })

        const rows = root.statementTransactionRows(root.session.selectedStatementId)
        const preferred = root.editTransactionOrderIds && root.editTransactionOrderIds.length > 0
            ? root.editTransactionOrderIds
            : root.sessionState.rowIds(rows)
        return root.sessionState.orderedSelectionState(rows,
                                                       preferred,
                                                       root.editTransactionIndex,
                                                       root.session.selectedTransactionId || "",
                                                       "id")
    }

    function transactionById(txId) {
        if (!root.workspaceFacade || !txId)
            return root.emptyTransaction()
        return root.cloneTransaction(root.workspaceFacade.transaction(txId))
    }

    function rememberSelectedTransaction() {
        if (!root.session || !root.session.selectedStatementId || !root.session.selectedTransactionId)
            return
        const statementId = String(root.session.selectedStatementId || "")
        const txId = String(root.session.selectedTransactionId || "")
        if (statementId.length === 0 || txId.length === 0)
            return
        const next = ({})
        const previous = root.lastTransactionIdByStatementId || ({})
        for (const key in previous)
            next[key] = previous[key]
        next[statementId] = txId
        root.lastTransactionIdByStatementId = next
    }

    function statementHasTransaction(statementId, txId) {
        const target = String(txId || "")
        if (target.length === 0)
            return false
        const rows = root.statementTransactionRows(statementId)
        for (let i = 0; i < rows.length; ++i) {
            if (String(rows[i].id || "") === target)
                return true
        }
        return false
    }

    function transactionIdForStatement(statementId) {
        const statementKey = String(statementId || "")
        const remembered = root.lastTransactionIdByStatementId
            ? String(root.lastTransactionIdByStatementId[statementKey] || "")
            : ""
        if (root.statementHasTransaction(statementKey, remembered))
            return remembered
        const rows = root.statementTransactionRows(statementKey)
        return rows.length > 0 && rows[0].id ? String(rows[0].id) : ""
    }

    function ensureSelectedTransactionForStatement() {
        if (root.isCreateMode || !root.session || !root.session.selectedStatementId)
            return true
        const statementId = String(root.session.selectedStatementId || "")
        const currentTxId = String(root.session.selectedTransactionId || "")
        if (root.statementHasTransaction(statementId, currentTxId))
            return true
        const nextTxId = root.transactionIdForStatement(statementId)
        if (currentTxId === nextTxId)
            return true
        root.session.selectedTransactionId = nextTxId
        return false
    }

    function currentCreateTransaction() {
        const list = root.createTransactions && root.createTransactions.length > 0
            ? root.createTransactions.slice()
            : [root.emptyTransaction()]
        const lastIndex = Math.max(0, list.length - 1)
        const index = Math.max(0, Math.min(root.createTransactionIndex, lastIndex))
        root.createTransactions = list
        root.createTransactionIndex = index
        root.currentTransactionDraft = list[index] || root.emptyTransaction()
        return root.currentTransactionDraft
    }

    function setCurrentCreateTransaction(data) {
        const list = root.createTransactions && root.createTransactions.length > 0
            ? root.createTransactions.slice()
            : [root.emptyTransaction()]
        const lastIndex = Math.max(0, list.length - 1)
        const index = Math.max(0, Math.min(root.createTransactionIndex, lastIndex))
        list[index] = data || root.emptyTransaction()
        root.createTransactions = list
        root.createTransactionIndex = index
        root.currentTransactionDraft = list[index] || root.emptyTransaction()
    }

    function syncEditState() {
        if (root.isCreateMode)
            return

        if (!root.ensureSelectedTransactionForStatement())
            return

        const statement = root.session ? root.session.selectedStatement : null
        root.editStatementName = statement && statement.name ? statement.name : ""

        const state = root.editTransactionState()
        const rows = state.rows || []
        root.editTransactionOrderIds = state.orderIds || []
        if (rows.length === 0) {
            root.editTransactionIndex = -1
            root.editTransactionData = root.emptyTransaction()
            return
        }

        root.editTransactionIndex = state.index !== undefined ? state.index : 0
        root.editTransactionData = root.transactionById(state.id || "")

        if (root.session)
            root.session.selectedTransactionId = state.id || ""
        root.rememberSelectedTransaction()
        root.captureEditState()
    }

    function navigateStatement(delta) {
        const _workspaceRevision = root.workspaceRevision
        const rows = root.statementRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isCreateMode ? "" : (root.session.selectedStatementId || "")
        const currentIndex = root.sessionState.indexOfId ? root.sessionState.indexOfId(rows, currentId) : -1
        if ((delta > 0 && currentIndex === rows.length - 1)
                || (delta < 0 && currentIndex === 0)) {
            root.rememberSelectedTransaction()
            root.session.selectedStatementId = ""
            root.session.selectedTransactionId = ""
            root.editTransactionOrderIds = []
            root.editTransactionIndex = -1
            return
        }
        const nextIndex = currentIndex < 0
            ? (delta > 0 ? 0 : rows.length - 1)
            : currentIndex + delta
        const nextId = rows[nextIndex] && rows[nextIndex].id ? String(rows[nextIndex].id) : ""
        if (!nextId || nextId.length === 0)
            return

        root.rememberSelectedTransaction()
        root.session.selectedStatementId = nextId
        root.session.selectedTransactionId = root.transactionIdForStatement(nextId)
        root.editTransactionOrderIds = []
        root.editTransactionIndex = 0
    }

    function navigateTransaction(delta) {
        const _workspaceRevision = root.workspaceRevision
        if (root.isCreateMode) {
            if (root.createTransactions.length === 0)
                return

            const idx = root.sessionState ? root.sessionState.wrappedIndex(root.createTransactionIndex + delta, root.createTransactions.length) : 0
            root.createTransactionIndex = idx
            return
        }

        const state = root.editTransactionState()
        root.editTransactionOrderIds = state.orderIds || []
        const rows = state.rows || []
        if (!root.session || rows.length === 0)
            return

        const next = root.sessionState.navigateSelectionState(rows,
                                                              state.index !== undefined ? state.index : root.editTransactionIndex,
                                                              state.id || root.session.selectedTransactionId || "",
                                                              delta,
                                                              0,
                                                              "id")
        root.editTransactionIndex = next.index !== undefined ? next.index : 0
        root.editTransactionData = root.transactionById(next.id || "")
        root.session.selectedTransactionId = next.id || ""
        root.rememberSelectedTransaction()
    }

    function resetCreateState() {
        root.createStatementName = ""
        if (!root.session) {
            root.createTransactions = [root.emptyTransaction()]
            root.createTransactionIndex = 0
            root.currentTransactionDraft = root.emptyTransaction()
            return
        }
        const state = root.sessionState.createDraftListState([], 0, root.emptyTransaction())
        root.createTransactions = state.drafts || [root.emptyTransaction()]
        root.createTransactionIndex = state.index !== undefined ? state.index : 0
        root.currentTransactionDraft = state.draft || root.emptyTransaction()
    }

    function addCreateTransaction() {
        if (!root.session) {
            return
        }

        const list = root.createTransactions && root.createTransactions.length > 0
            ? root.createTransactions.slice()
            : [root.emptyTransaction()]
        const index = Math.max(0, Math.min(root.createTransactionIndex, list.length - 1))
        const insertIndex = Math.max(0, Math.min(index + 1, list.length))
        list.splice(insertIndex, 0, root.emptyTransaction())
        root.createTransactions = list
        root.createTransactionIndex = insertIndex
        root.currentTransactionDraft = list[insertIndex] || root.emptyTransaction()
    }

    function addEditTransaction() {
        if (!root.workspaceFacade || !root.session || !root.session.selectedStatementId)
            return

        const state = root.editTransactionState()
        root.editTransactionOrderIds = state.orderIds || []
        const rows = state.rows || []
        let insertAfterIndex = state.index !== undefined ? state.index : root.editTransactionIndex
        if (insertAfterIndex < 0 || insertAfterIndex >= rows.length)
            insertAfterIndex = rows.length - 1

        const statementId = root.session.selectedStatementId
        const current = root.cloneTransaction(root.editTransactionData || root.emptyTransaction())
        const bookingDate = current.bookingDate || ""
        const valuta = current.valuta || ""
        const afterTransactionId = current.id || (state.id || "")
        const newId = root.workspaceFacade.insertTransactionAfter(afterTransactionId, "", bookingDate, valuta, 0.0, statementId, 0, "", "", false, [])
        if (!newId || newId.length === 0)
            return

        const updatedRows = root.statementTransactionRows(statementId)
        const updatedIds = root.sessionState.rowIds(updatedRows)

        root.editTransactionOrderIds = root.sessionState
            ? root.sessionState.orderWithInsertedId(root.editTransactionOrderIds || [], updatedIds, newId, insertAfterIndex)
            : updatedIds

        if (root.session)
            root.session.selectedTransactionId = newId

        Qt.callLater(root.syncEditState)
    }

    function addTransactionAfterCurrent() {
        if (root.isCreateMode)
            root.addCreateTransaction()
        else
            root.addEditTransaction()
    }

    function deleteCurrentTransaction() {
        if (root.isCreateMode) {
            if (!root.session)
                return
            const list = root.createTransactions && root.createTransactions.length > 0
                ? root.createTransactions.slice()
                : [root.emptyTransaction()]
            const index = Math.max(0, Math.min(root.createTransactionIndex, list.length - 1))
            if (list.length > 1)
                list.splice(index, 1)
            if (list.length === 0)
                list.push(root.emptyTransaction())
            const nextIndex = Math.max(0, Math.min(index, list.length - 1))
            root.createTransactions = list
            root.createTransactionIndex = nextIndex
            root.currentTransactionDraft = list[nextIndex] || root.emptyTransaction()
            return
        }

        if (!root.workspaceFacade || !root.editTransactionData || !root.editTransactionData.id)
            return

        const selectionState = root.editTransactionState()
        root.editTransactionOrderIds = selectionState.orderIds || []
        const rows = selectionState.rows || []
        if (rows.length <= 1)
            return

        const deletedId = root.editTransactionData.id
        root.workspaceFacade.deleteTransaction(root.editTransactionData.id)

        const updatedRows = root.statementTransactionRows(root.session.selectedStatementId || "")
        const reselectionState = root.sessionState
            ? root.sessionState.deleteReselectionState(updatedRows,
                                                       root.editTransactionOrderIds || [],
                                                       root.editTransactionIndex,
                                                       deletedId,
                                                       "id")
            : ({ orderIds: [], index: -1, id: "" })

        root.editTransactionOrderIds = reselectionState.orderIds || []
        root.editTransactionIndex = reselectionState.index !== undefined ? reselectionState.index : -1
        if (root.session)
            root.session.selectedTransactionId = reselectionState.id || ""
        root.rememberSelectedTransaction()
        root.editTransactionData = root.transactionById(reselectionState.id || "")
    }

    function transactionInfoText() {
        if (root.isCreateMode)
            return root.createTransactions.length === 0
                ? qsTr("No transactions")
                : qsTr("Transaction %1 / %2").arg(root.createTransactionIndex + 1).arg(root.createTransactions.length)

        const state = root.editTransactionState()
        const rows = state.rows || []
        root.editTransactionOrderIds = state.orderIds || []
        if (rows.length === 0)
            return qsTr("No transactions")
        const idx = state.index !== undefined ? state.index : 0
        return qsTr("Transaction %1 / %2").arg(idx + 1).arg(rows.length)
    }

    function createStatementWithTransactions() {
        if (!root.workspaceFacade || !root.workspaceFacade)
            return
        if (!root.createStatementName || root.createStatementName.length === 0)
            return

        const statementId = root.workspaceFacade.addStatement(root.createStatementName)
        if (!statementId || statementId.length === 0)
            return

        const drafts = root.createTransactions || []
        for (let i = 0; i < drafts.length; ++i) {
            const rawTx = drafts[i] || root.emptyTransaction()
            const tx = root.cloneTransaction(rawTx)
            root.workspaceFacade.addTransaction(
                tx.name || "",
                tx.bookingDate || "",
                tx.valuta || "",
                root.amountForCommit(rawTx.amount, "", tx.amount),
                statementId,
                tx.status !== undefined ? Number(tx.status) : 0,
                tx.actorId || "",
                tx.contractId || "",
                !!tx.allocatable,
                tx.propertyIds || [])
        }

        root.resetCreateState()
        if (root.session) {
            root.session.selectedStatementId = statementId
            root.session.selectedTransactionId = root.transactionIdForStatement(statementId)
        }
    }

    function updateSelectedEntity() {
        if (root.isCreateMode)
            return

        if (!root.workspaceFacade || !root.session || !root.session.selectedStatementId)
            return

        const selectedStatementId = root.session.selectedStatementId || ""
        const currentStatementName = String(root.editStatementName || "")
        const statementChanged = root.savedEditStatementName !== currentStatementName

        const txData = root.editTransactionData || root.emptyTransaction()
        const txId = txData.id || ""
        const transactionChanged = root.savedEditTransactionJson !== root.transactionSnapshot(txData)

        const normalizedTx = root.cloneTransaction(txData)
        const txName = txData.name || ""
        const txBookingDate = txData.bookingDate || ""
        const txValuta = txData.valuta || ""
        const txAmount = root.amountForCommit(txData.amount, txId, normalizedTx.amount)
        const txStatus = normalizedTx.status
        const txActorId = txData.actorId || ""
        const txContractId = txData.contractId || ""
        const txAllocatable = !!txData.allocatable
        const txPropertyIds = txData.propertyIds || []
        const txStatementId = selectedStatementId || (txData.statementId || "")

        if (transactionChanged && txId.length > 0) {
            root.workspaceFacade.updateTransaction(
                txId,
                txName,
                txBookingDate,
                txValuta,
                txAmount,
                txStatementId,
                txStatus,
                txActorId,
                txContractId,
                txAllocatable,
                txPropertyIds)
        }

        if (statementChanged) {
            root.workspaceFacade.updateStatement(selectedStatementId, currentStatementName)
        }

        Qt.callLater(root.syncEditState)
    }

    function deleteStatement() {
        if (root.isCreateMode)
            return
        if (!root.workspaceFacade || !root.session || !root.session.selectedStatementId)
            return

        const removedId = root.session.selectedStatementId
        root.workspaceFacade.deleteStatement(removedId)
        const nextId = root.sessionState.deleteNextSelectionId(root.statementRows(), removedId, 0, "id")
        root.session.selectedStatementId = nextId || ""
        root.session.selectedTransactionId = nextId ? root.transactionIdForStatement(nextId) : ""
    }

    Connections {
        target: root.session

        function onSelectedStatementIdChanged() {
            if (root.isCreateMode)
                root.resetCreateState()
            else {
                root.editTransactionOrderIds = []
                root.editTransactionIndex = 0
                root.syncEditState()
            }
        }

        function onSelectedTransactionIdChanged() {
            if (root.isCreateMode)
                root.resetCreateState()
            else {
                root.rememberSelectedTransaction()
                root.syncEditState()
            }
        }

        function onDataRevisionChanged() {
            if (root.isCreateMode)
                root.resetCreateState()
            else
                root.syncEditState()
        }
    }

    Component.onCompleted: {
        if (root.isCreateMode)
            root.resetCreateState()
        else
            root.syncEditState()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        BookingStatementPanel {
            id: statementPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            statementName: root.isCreateMode ? root.createStatementName : root.editStatementName
            transactionInfoText: root.isCreateMode
                                 ? (root.createTransactions.length === 0 ? qsTr("No transactions") : qsTr("Transaction %1 / %2").arg(root.createTransactionIndex + 1).arg(root.createTransactions.length))
                                 : (function() {
                                     const state = root.editTransactionState()
                                     const rows = state.rows || []
                                     if (rows.length === 0)
                                         return qsTr("No transactions")
                                     const idx = state.index !== undefined ? state.index : 0
                                     return qsTr("Transaction %1 / %2").arg(idx + 1).arg(rows.length)
                                 })()
            transactionDeleteVisible: true
            transactionDeleteEnabled: root.isCreateMode
                                      ? root.createTransactions.length > 1
                                      : (root.editTransactionState().rows.length > 1 && root.editTransactionData && root.editTransactionData.id && root.editTransactionData.id.length > 0)
            transactionAddVisible: root.isCreateMode || (!root.isCreateMode && root.session && root.session.selectedStatementId && root.session.selectedStatementId.length > 0)
            transactionAddEnabled: root.isCreateMode || (!root.isCreateMode && root.session && root.session.selectedStatementId && root.session.selectedStatementId.length > 0)
            onStatementNameEdited: function(nextText) {
                if (root.isCreateMode)
                    root.createStatementName = nextText
                else
                    root.editStatementName = nextText
            }
            onTransactionAddRequested: root.addTransactionAfterCurrent()
            onTransactionDeleteRequested: root.deleteCurrentTransaction()

            BookingTransactionView {
                id: transactionView
                Layout.fillWidth: true
                theme: root.theme
                sessionState: root.sessionState
                transactionData: root.isCreateMode ? root.currentTransactionDraft : root.editTransactionData
                actorRows: root.actorRows()
                contractRows: root.contractRows()
                propertyRows: root.propertyRows()
                onTransactionEdited: function(nextData) {
                    if (root.isCreateMode)
                        root.setCurrentCreateTransaction(nextData)
                    else
                        root.editTransactionData = nextData || root.emptyTransaction()
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevPageButton {
                objectName: "bookingPreviousStatementButton"
                enabled: root.statementRows().length > 0
                onClicked: root.navigateStatement(-1)
            }

            Controls.PrevButton {
                objectName: "bookingPreviousTransactionButton"
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionState().rows.length > 1
                onClicked: root.navigateTransaction(-1)
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.DangerButton {
                objectName: "bookingClearButton"
                visible: root.isCreateMode
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.resetCreateState()
            }

            Controls.SuccessButton {
                objectName: "bookingCreateButton"
                visible: root.isCreateMode
                text: qsTr("Create")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                enabled: root.createStatementName.length > 0
                onClicked: root.createStatementWithTransactions()
            }

            Controls.DangerButton {
                objectName: "bookingDeleteButton"
                visible: !root.isCreateMode
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteStatement()
            }

            Controls.SuccessButton {
                objectName: "bookingUpdateButton"
                visible: !root.isCreateMode
                text: qsTr("Update")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                enabled: root.hasEditChanges()
                onClicked: root.updateSelectedEntity()
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.NextButton {
                objectName: "bookingNextTransactionButton"
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionState().rows.length > 1
                onClicked: root.navigateTransaction(1)
            }

            Controls.NextPageButton {
                objectName: "bookingNextStatementButton"
                enabled: root.statementRows().length > 0
                onClicked: root.navigateStatement(1)
            }
        }
    }
}
