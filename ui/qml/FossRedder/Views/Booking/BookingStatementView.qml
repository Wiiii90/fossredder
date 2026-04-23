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
        return {
            id: "",
            name: "",
            bookingDate: "",
            valuta: "",
            amount: 0.0,
            description: "",
            status: 0,
            actorId: "",
            propertyIds: [],
            allocatable: false,
            contractId: "",
            statementId: ""
        }
    }

    function cloneTransaction(tx) {
        const base = root.emptyTransaction()
        const src = tx || ({})
        for (const key in src)
            base[key] = src[key]

        const ids = []
        if (src && src.propertyIds && src.propertyIds.length !== undefined) {
            for (let i = 0; i < src.propertyIds.length; ++i)
                ids.push(String(src.propertyIds[i]))
        }
        base.propertyIds = ids
        base.amount = Number(base.amount)
        if (isNaN(base.amount)) base.amount = 0.0
        base.status = Number(base.status)
        if (isNaN(base.status)) base.status = 0
        return base
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

    function selectedStatementIndex() {
        if (!root.session || !root.session.selectedStatementId)
            return -1

        const rows = root.statementRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === root.session.selectedStatementId)
                return i
        }
        return -1
    }

    function editTransactionRows() {
        if (!root.session || !root.session.selectedStatementId)
            return []

        const rows = root.session.statementTransactionRows(root.session.selectedStatementId)
        if (rows.length === 0) {
            root.editTransactionOrderIds = []
            return []
        }

        const byId = ({})
        const ids = []
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (!row || !row.id)
                continue
            byId[row.id] = row
            ids.push(row.id)
        }

        let order = root.editTransactionOrderIds && root.editTransactionOrderIds.length > 0
            ? root.editTransactionOrderIds.slice()
            : ids.slice()

        order = order.filter(function(id) { return ids.indexOf(id) !== -1 })
        for (let i = 0; i < ids.length; ++i) {
            if (order.indexOf(ids[i]) === -1)
                order.push(ids[i])
        }
        root.editTransactionOrderIds = order

        const ordered = []
        for (let i = 0; i < order.length; ++i) {
            if (byId[order[i]])
                ordered.push(byId[order[i]])
        }
        return ordered
    }

    function transactionById(txId) {
        if (!root.transactionController || !txId)
            return root.emptyTransaction()
        return root.cloneTransaction(root.transactionController.transaction(txId))
    }

    function currentCreateTransaction() {
        if (root.createTransactions.length === 0)
            return root.emptyTransaction()
        const idx = Math.max(0, Math.min(root.createTransactionIndex, root.createTransactions.length - 1))
        return root.cloneTransaction(root.createTransactions[idx])
    }

    function setCurrentCreateTransaction(data) {
        if (root.createTransactions.length === 0)
            return

        const idx = Math.max(0, Math.min(root.createTransactionIndex, root.createTransactions.length - 1))
        let next = root.createTransactions.slice()
        next[idx] = root.cloneTransaction(data)
        root.createTransactions = next
    }

    function syncEditState() {
        if (root.isCreateMode)
            return

        const statement = root.session ? root.session.selectedStatement : null
        root.editStatementName = statement && statement.name ? statement.name : ""

        const rows = root.editTransactionRows()
        if (rows.length === 0) {
            root.editTransactionIndex = -1
            root.editTransactionData = root.emptyTransaction()
            return
        }

        let index = root.editTransactionIndex
        if (root.session && root.session.selectedTransactionId && root.session.selectedTransactionId.length > 0) {
            index = -1
            for (let i = 0; i < rows.length; ++i) {
                if (rows[i] && rows[i].id === root.session.selectedTransactionId) {
                    index = i
                    break
                }
            }
        }

        if (index < 0 || index >= rows.length)
            index = 0

        root.editTransactionIndex = index
        const currentRow = rows[index]
        root.editTransactionData = root.transactionById(currentRow && currentRow.id ? currentRow.id : "")

        if (root.session)
            root.session.selectedTransactionId = currentRow && currentRow.id ? currentRow.id : ""
    }

    function selectStatementByIndex(index) {
        const rows = root.statementRows()
        if (!root.session || rows.length === 0)
            return

        let idx = index
        while (idx < 0)
            idx += rows.length
        idx = idx % rows.length

        const row = rows[idx]
        if (!row || !row.id)
            return

        root.session.selectedStatementId = row.id
        root.session.selectedTransactionId = ""
        root.editTransactionOrderIds = []
    }

    function navigateStatement(delta) {
        const rows = root.statementRows()
        if (rows.length === 0)
            return

        if (root.isCreateMode) {
            root.selectStatementByIndex(delta > 0 ? 0 : rows.length - 1)
            return
        }

        const current = root.selectedStatementIndex()
        if (current < 0)
            return

        root.selectStatementByIndex(current + delta)
    }

    function navigateTransaction(delta) {
        if (root.isCreateMode) {
            if (root.createTransactions.length === 0)
                return

            let idx = root.createTransactionIndex + delta
            while (idx < 0)
                idx += root.createTransactions.length
            idx = idx % root.createTransactions.length
            root.createTransactionIndex = idx
            return
        }

        const rows = root.editTransactionRows()
        if (!root.session || rows.length === 0)
            return

        let idx = root.editTransactionIndex
        if (idx < 0 || idx >= rows.length)
            idx = 0
        idx = idx + delta
        while (idx < 0)
            idx += rows.length
        idx = idx % rows.length

        const row = rows[idx]
        root.editTransactionIndex = idx
        root.editTransactionData = root.transactionById(row && row.id ? row.id : "")
        root.session.selectedTransactionId = row && row.id ? row.id : ""
    }

    function resetCreateState() {
        root.createStatementName = ""
        root.createTransactions = [root.emptyTransaction()]
        root.createTransactionIndex = 0
    }

    function addCreateTransaction() {
        if (root.createTransactions.length === 0) {
            root.createTransactions = [root.emptyTransaction()]
            root.createTransactionIndex = 0
            return
        }

        const next = root.createTransactions.slice()
        const insertIndex = Math.max(0, Math.min(root.createTransactionIndex, next.length - 1)) + 1
        next.splice(insertIndex, 0, root.emptyTransaction())
        root.createTransactions = next
        root.createTransactionIndex = insertIndex
    }

    function addEditTransaction() {
        if (!root.transactionController || !root.session || !root.session.selectedStatementId)
            return

        const rows = root.editTransactionRows()
        let insertAfterIndex = root.editTransactionIndex
        if (insertAfterIndex < 0 || insertAfterIndex >= rows.length)
            insertAfterIndex = rows.length - 1

        const statementId = root.session.selectedStatementId
        const newId = root.transactionController.addTransaction("", "", 0.0, "", statementId, 0, "", false, [])
        if (!newId || newId.length === 0)
            return

        const updatedRows = root.session.statementTransactionRows(statementId)
        const updatedIds = []
        for (let i = 0; i < updatedRows.length; ++i) {
            if (updatedRows[i] && updatedRows[i].id)
                updatedIds.push(updatedRows[i].id)
        }

        let order = root.editTransactionOrderIds && root.editTransactionOrderIds.length > 0
            ? root.editTransactionOrderIds.slice()
            : updatedIds.slice()
        order = order.filter(function(id) { return updatedIds.indexOf(id) !== -1 && id !== newId })
        const insertIndex = Math.max(0, Math.min(insertAfterIndex + 1, order.length))
        order.splice(insertIndex, 0, newId)
        for (let i = 0; i < updatedIds.length; ++i) {
            if (order.indexOf(updatedIds[i]) === -1)
                order.push(updatedIds[i])
        }
        root.editTransactionOrderIds = order

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
            if (root.createTransactions.length <= 1) {
                root.createTransactions = [root.emptyTransaction()]
                root.createTransactionIndex = 0
                return
            }

            let next = root.createTransactions.slice()
            next.splice(root.createTransactionIndex, 1)
            root.createTransactions = next
            root.createTransactionIndex = Math.min(root.createTransactionIndex, next.length - 1)
            return
        }

        if (!root.transactionController || !root.editTransactionData || !root.editTransactionData.id)
            return

        const rows = root.editTransactionRows()
        if (rows.length <= 1)
            return

        const deletedId = root.editTransactionData.id
        root.transactionController.deleteTransaction(root.editTransactionData.id)
        root.editTransactionOrderIds = root.editTransactionOrderIds.filter(function(id) { return id !== deletedId })
        if (root.session)
            root.session.selectedTransactionId = ""
        Qt.callLater(root.syncEditState)
    }

    function currentCreateInfoText() {
        if (root.createTransactions.length === 0)
            return qsTr("No transactions")
        return qsTr("Transaction %1 / %2").arg(root.createTransactionIndex + 1).arg(root.createTransactions.length)
    }

    function currentEditInfoText() {
        const rows = root.editTransactionRows()
        if (rows.length === 0)
            return qsTr("No transactions")
        const idx = root.editTransactionIndex >= 0 ? root.editTransactionIndex : 0
        return qsTr("Transaction %1 / %2").arg(idx + 1).arg(rows.length)
    }

    function createStatementWithTransactions() {
        if (!root.statementController)
            return
        if (!root.createStatementName || root.createStatementName.length === 0)
            return

        const statementId = root.statementController.addStatement(root.createStatementName)
        if (!statementId || statementId.length === 0)
            return

        if (root.transactionController) {
            for (let i = 0; i < root.createTransactions.length; ++i) {
                const tx = root.cloneTransaction(root.createTransactions[i])
                const hasContent = (tx.name && tx.name.length > 0)
                                   || (tx.bookingDate && tx.bookingDate.length > 0)
                                   || (tx.valuta && tx.valuta.length > 0)
                                   || Number(tx.amount) !== 0
                                   || (tx.actorId && tx.actorId.length > 0)
                                   || (tx.propertyIds && tx.propertyIds.length > 0)
                if (!hasContent)
                    continue

                root.transactionController.addTransaction(
                    tx.name || "",
                    tx.bookingDate || "",
                    Number(tx.amount),
                    tx.description || "",
                    statementId,
                    Number(tx.status),
                    tx.actorId || "",
                    !!tx.allocatable,
                    tx.propertyIds || [])
            }
        }

        root.resetCreateState()
        if (root.session) {
            root.session.selectedStatementId = statementId
            root.session.selectedTransactionId = ""
        }
    }

    function updateSelectedEntity() {
        if (root.isCreateMode)
            return

        if (root.statementController && root.session && root.session.selectedStatementId)
            root.statementController.updateStatement(root.session.selectedStatementId, root.editStatementName)

        if (!root.transactionController || !root.editTransactionData || !root.editTransactionData.id)
            return

        const statementId = root.session && root.session.selectedStatementId ? root.session.selectedStatementId : (root.editTransactionData.statementId || "")
        root.transactionController.updateTransaction(
            root.editTransactionData.id,
            root.editTransactionData.name || "",
            root.editTransactionData.bookingDate || "",
            Number(root.editTransactionData.amount),
            root.editTransactionData.description || "",
            statementId,
            Number(root.editTransactionData.status),
            root.editTransactionData.actorId || "",
            !!root.editTransactionData.allocatable,
            root.editTransactionData.propertyIds || [])
    }

    function deleteStatement() {
        if (root.isCreateMode)
            return
        if (!root.statementController || !root.session || !root.session.selectedStatementId)
            return

        root.statementController.deleteStatement(root.session.selectedStatementId)
        root.session.selectedStatementId = ""
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
                                      : (root.editTransactionRows().length > 1 && root.editTransactionData && root.editTransactionData.id && root.editTransactionData.id.length > 0)
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
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionRows().length > 1
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
                enabled: root.isCreateMode ? root.createTransactions.length > 1 : root.editTransactionRows().length > 1
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
