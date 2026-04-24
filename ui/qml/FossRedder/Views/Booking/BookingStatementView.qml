/*!
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementView.qml
 * @brief Main booking page for creating and editing statements and their transactions.
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
    readonly property var statementController: root.appContext ? root.appContext.statementController : null
    readonly property var transactionController: root.appContext ? root.appContext.transactionController : null

    property string createStatementName: ""
    property var createTransactions: [root.emptyTransaction()]
    property int createTransactionIndex: 0

    property string editStatementName: ""
    property var editTransactionData: root.emptyTransaction()
    property int editTransactionIndex: -1
    property var editTransactionOrderIds: []

    readonly property bool isCreateMode: !root.session || !root.session.selectedStatementId || root.session.selectedStatementId.length === 0

    function emptyTransaction() {
        return root.session ? root.session.emptyTransactionDraft() : ({})
    }

    function cloneTransaction(tx) {
        return root.session ? root.session.normalizeTransactionDraft(tx || ({})) : ({})
    }

    function statementRows() {
        return root.session ? root.session.statementRows() : []
    }

    function actorRows() {
        return root.session ? root.session.actorRows() : []
    }

    function contractRows() {
        return root.session ? root.session.contractRows() : []
    }

    function propertyRows() {
        return root.session ? root.session.propertyRows() : []
    }

    function editTransactionState() {
        if (!root.session || !root.session.selectedStatementId)
            return ({ rows: [], orderIds: [], index: -1, id: "" })

        const rows = root.session.statementTransactionRows(root.session.selectedStatementId)
        const preferred = root.editTransactionOrderIds && root.editTransactionOrderIds.length > 0
            ? root.editTransactionOrderIds
            : root.session.rowIds(rows)
        return root.session.orderedSelectionState(rows,
                                                  preferred,
                                                  root.editTransactionIndex,
                                                  root.session.selectedTransactionId || "",
                                                  "id")
    }

    function transactionById(txId) {
        if (!root.statementController || !txId)
            return root.emptyTransaction()
        return root.cloneTransaction(root.statementController.statement(txId))
    }

    function currentCreateTransaction() {
        if (!root.session)
            return root.emptyTransaction()
        const state = root.session.currentDraftState(root.createTransactions || [],
                                                     root.createTransactionIndex,
                                                     root.emptyTransaction())
        root.createTransactions = state.drafts || [root.emptyTransaction()]
        root.createTransactionIndex = state.index !== undefined ? state.index : 0
        return state.draft || root.emptyTransaction()
    }

    function setCurrentCreateTransaction(data) {
        if (!root.session)
            return

        const state = root.session.setCurrentDraft(root.createTransactions || [],
                                                   root.createTransactionIndex,
                                                   data || ({}),
                                                   root.emptyTransaction())
        root.createTransactions = state.drafts || [root.emptyTransaction()]
        root.createTransactionIndex = state.index !== undefined ? state.index : 0
    }

    function syncEditState() {
        if (root.isCreateMode)
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
    }

    function navigateStatement(delta) {
        const rows = root.statementRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isCreateMode ? "" : (root.session.selectedStatementId || "")
        const fallbackIndex = delta > 0 ? 0 : rows.length - 1
        const nextId = root.session.navigatedId(rows, currentId, delta, fallbackIndex)
        if (!nextId || nextId.length === 0)
            return

        root.session.selectedStatementId = nextId
        root.session.selectedTransactionId = ""
        root.editTransactionOrderIds = []
    }

    function navigateTransaction(delta) {
        if (root.isCreateMode) {
            if (root.createTransactions.length === 0)
                return

            const idx = root.session ? root.session.wrappedIndex(root.createTransactionIndex + delta, root.createTransactions.length) : 0
            root.createTransactionIndex = idx
            return
        }

        const state = root.editTransactionState()
        root.editTransactionOrderIds = state.orderIds || []
        const rows = state.rows || []
        if (!root.session || rows.length === 0)
            return

        const next = root.session.navigateSelectionState(rows,
                                                         state.index !== undefined ? state.index : root.editTransactionIndex,
                                                         state.id || root.session.selectedTransactionId || "",
                                                         delta,
                                                         0,
                                                         "id")
        root.editTransactionIndex = next.index !== undefined ? next.index : 0
        root.editTransactionData = root.transactionById(next.id || "")
        root.session.selectedTransactionId = next.id || ""
    }

    function resetCreateState() {
        root.createStatementName = ""
        if (!root.session) {
            root.createTransactions = [root.emptyTransaction()]
            root.createTransactionIndex = 0
            return
        }
        const state = root.session.createDraftListState([], 0, root.emptyTransaction())
        root.createTransactions = state.drafts || [root.emptyTransaction()]
        root.createTransactionIndex = state.index !== undefined ? state.index : 0
    }

    function addCreateTransaction() {
        if (!root.session) {
            return
        }

        const state = root.session.insertDraftAfterCurrent(root.createTransactions || [],
                                                           root.createTransactionIndex,
                                                           root.emptyTransaction())
        root.createTransactions = state.drafts || [root.emptyTransaction()]
        root.createTransactionIndex = state.index !== undefined ? state.index : 0
    }

    function addEditTransaction() {
        if (!root.transactionController || !root.session || !root.session.selectedStatementId)
            return

        const state = root.editTransactionState()
        root.editTransactionOrderIds = state.orderIds || []
        const rows = state.rows || []
        let insertAfterIndex = state.index !== undefined ? state.index : root.editTransactionIndex
        if (insertAfterIndex < 0 || insertAfterIndex >= rows.length)
            insertAfterIndex = rows.length - 1

        const statementId = root.session.selectedStatementId
        const newId = root.transactionController.addTransaction("", "", 0.0, "", statementId, 0, "", false, [])
        if (!newId || newId.length === 0)
            return

        const updatedRows = root.session.statementTransactionRows(statementId)
        const updatedIds = root.session.rowIds(updatedRows)

        root.editTransactionOrderIds = root.session
            ? root.session.orderWithInsertedId(root.editTransactionOrderIds || [], updatedIds, newId, insertAfterIndex)
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
            const state = root.session.removeDraftAt(root.createTransactions || [],
                                                     root.createTransactionIndex,
                                                     root.emptyTransaction())
            root.createTransactions = state.drafts || [root.emptyTransaction()]
            root.createTransactionIndex = state.index !== undefined ? state.index : 0
            return
        }

        if (!root.transactionController || !root.editTransactionData || !root.editTransactionData.id)
            return

        const selectionState = root.editTransactionState()
        root.editTransactionOrderIds = selectionState.orderIds || []
        const rows = selectionState.rows || []
        if (rows.length <= 1)
            return

        const deletedId = root.editTransactionData.id
        root.transactionController.deleteTransaction(root.editTransactionData.id)

        const updatedRows = root.session ? root.session.statementTransactionRows(root.session.selectedStatementId || "") : []
        const reselectionState = root.session
            ? root.session.deleteReselectionState(updatedRows,
                                                 root.editTransactionOrderIds || [],
                                                 root.editTransactionIndex,
                                                 deletedId,
                                                 "id")
            : ({ orderIds: [], index: -1, id: "" })

        root.editTransactionOrderIds = reselectionState.orderIds || []
        root.editTransactionIndex = reselectionState.index !== undefined ? reselectionState.index : -1
        if (root.session)
            root.session.selectedTransactionId = reselectionState.id || ""
        root.editTransactionData = root.transactionById(reselectionState.id || "")
    }

    function currentCreateInfoText() {
        if (root.createTransactions.length === 0)
            return qsTr("No transactions")
        return qsTr("Transaction %1 / %2").arg(root.createTransactionIndex + 1).arg(root.createTransactions.length)
    }

    function currentEditInfoText() {
        const state = root.editTransactionState()
        const rows = state.rows || []
        root.editTransactionOrderIds = state.orderIds || []
        if (rows.length === 0)
            return qsTr("No transactions")
        const idx = state.index !== undefined ? state.index : 0
        return qsTr("Transaction %1 / %2").arg(idx + 1).arg(rows.length)
    }

    function createStatementWithTransactions() {
        if (!root.statementController || !root.transactionController)
            return
        if (!root.createStatementName || root.createStatementName.length === 0)
            return

        const statementId = root.statementController.addStatement(root.createStatementName)
        if (!statementId || statementId.length === 0)
            return

        root.transactionController.addTransactions(statementId, root.createTransactions || [])

        root.resetCreateState()
        if (root.session) {
            root.session.selectedStatementId = statementId
            root.session.selectedTransactionId = ""
        }
    }

    function updateSelectedEntity() {
        if (root.isCreateMode)
            return

        if (!root.statementController || !root.session || !root.session.selectedStatementId)
            return

        root.statementController.updateStatement(root.session.selectedStatementId,
                                                root.editStatementName)

        if (!root.transactionController || !root.editTransactionData || !root.editTransactionData.id)
            return

        const normalizedTx = root.cloneTransaction(root.editTransactionData)
        const statementId = root.session.selectedStatementId || (root.editTransactionData.statementId || "")
        root.transactionController.updateTransaction(
            root.editTransactionData.id,
            root.editTransactionData.name || "",
            root.editTransactionData.bookingDate || "",
            normalizedTx.amount,
            root.editTransactionData.description || "",
            statementId,
            normalizedTx.status,
            root.editTransactionData.actorId || "",
            !!root.editTransactionData.allocatable,
            root.editTransactionData.propertyIds || [])
    }

    function deleteStatement() {
        if (root.isCreateMode)
            return
        if (!root.statementController || !root.session || !root.session.selectedStatementId)
            return

        const removedId = root.session.selectedStatementId
        root.statementController.deleteStatement(removedId)
        const nextId = root.session.deleteNextSelectionId(root.statementRows(), removedId, 0, "id")
        root.session.selectedStatementId = nextId || ""
        root.session.selectedTransactionId = ""
    }

    Connections {
        target: root.session

        function onSelectedStatementIdChanged() {
            if (!root.isCreateMode)
                root.syncEditState()
        }

        function onSelectedTransactionIdChanged() {
            if (!root.isCreateMode)
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
        anchors.margins: 12
        spacing: root.theme.spacingSmall

        BookingStatementPanel {
            id: statementPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            statementName: root.isCreateMode ? root.createStatementName : root.editStatementName
            transactionInfoText: root.isCreateMode ? root.currentCreateInfoText() : root.currentEditInfoText()
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
                session: root.session
                transactionData: root.isCreateMode ? root.currentCreateTransaction() : root.editTransactionData
                actorRows: root.actorRows()
                contractRows: root.contractRows()
                propertyRows: root.propertyRows()
                onTransactionEdited: function(nextData) {
                    if (root.isCreateMode)
                        root.setCurrentCreateTransaction(nextData)
                    else
                        root.editTransactionData = root.cloneTransaction(nextData)
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                text: "⟪"
                enabled: root.statementRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateStatement(-1)
            }

            Controls.Button {
                text: "◀"
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionState().rows.length > 1
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateTransaction(-1)
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.DangerButton {
                visible: root.isCreateMode
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.resetCreateState()
            }

            Controls.SuccessButton {
                visible: root.isCreateMode
                text: qsTr("Create")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                enabled: root.createStatementName.length > 0
                onClicked: root.createStatementWithTransactions()
            }

            Controls.DangerButton {
                visible: !root.isCreateMode
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteStatement()
            }

            Controls.SuccessButton {
                visible: !root.isCreateMode
                text: qsTr("Update")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.updateSelectedEntity()
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: "▶"
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionState().rows.length > 1
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateTransaction(1)
            }

            Controls.Button {
                text: "⟫"
                enabled: root.statementRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateStatement(1)
            }
        }
    }
}
