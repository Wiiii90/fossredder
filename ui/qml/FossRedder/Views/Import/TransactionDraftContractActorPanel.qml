/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractActorPanel.qml
 * @brief Edits actor text and actor matching inside the transaction draft contract panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    property bool embedded: false
    property var localActorRows: []
    property string pendingActorText: ""
    property bool hasPendingActorEdit: false
    property string lastTxId: ""
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property real actorControlSize: root.theme.viewCompactActionButtonSize

    function sessionActorRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session && root.session.actorRows ? root.session.actorRows() : []
    }

    function rememberLocalActorRow(row) {
        if (!row || row.id === undefined || row.id === null)
            return
        const id = String(row.id)
        if (id.length === 0)
            return
        const rows = root.localActorRows ? root.localActorRows.slice() : []
        for (let i = 0; i < rows.length; ++i) {
            if (String(rows[i].id || "") === id) {
                rows[i] = row
                root.localActorRows = rows
                return
            }
        }
        rows.push(row)
        root.localActorRows = rows
    }

    function actorChoiceModel() {
        const rows = root.txRoot && root.txRoot.actorChoices ? root.txRoot.actorChoices : []
        const sessionRows = root.sessionActorRows()
        const model = [{ id: "", display: qsTr("No actor") }]
        const seenIds = {}
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (!row)
                continue
            const id = row.id !== undefined && row.id !== null ? String(row.id) : ""
            if (id.length === 0)
                continue
            seenIds[id] = true
            model.push({
                id: id,
                display: row.display !== undefined && row.display !== null ? String(row.display) : ""
            })
        }
        for (let i = 0; i < sessionRows.length; ++i) {
            const row = sessionRows[i]
            if (!row)
                continue
            const id = row.id !== undefined && row.id !== null ? String(row.id) : ""
            if (id.length === 0 || seenIds[id])
                continue
            seenIds[id] = true
            model.push({
                id: id,
                display: row.display !== undefined && row.display !== null
                    ? String(row.display)
                    : (row.name !== undefined && row.name !== null ? String(row.name) : "")
            })
        }
        const localRows = root.localActorRows || []
        for (let j = 0; j < localRows.length; ++j) {
            const localRow = localRows[j]
            if (!localRow)
                continue
            const localId = localRow.id !== undefined && localRow.id !== null ? String(localRow.id) : ""
            if (localId.length === 0 || seenIds[localId])
                continue
            model.push({
                id: localId,
                display: localRow.display !== undefined && localRow.display !== null ? String(localRow.display) : ""
            })
        }
        return model
    }

    function hasIdenticalActor() {
        const name = String(actorTextField.text || "").trim()
        if (name.length === 0)
            return false
        const target = normalizedText(name)
        const model = actorChoiceModel()
        for (let i = 0; i < model.length; ++i) {
            const row = model[i]
            if (!row || !row.id || String(row.id).length === 0)
                continue
            if (normalizedText(row.display) === target)
                return true
        }
        return false
    }

    function normalizedText(value) {
        return String(value || "").trim().replace(/\s+/g, " ").toLowerCase()
    }

    function selectedActorIndex(model) {
        const actorId = root.txRoot && root.txRoot.draft && root.txRoot.draft.current
            ? String(root.txRoot.draft.current.actorId || "")
            : ""
        for (let i = 0; i < model.length; ++i) {
            if (String(model[i].id || "") === actorId)
                return i
        }
        return 0
    }

    function addActorFromText() {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        const name = String(actorTextField.text || "").trim()
        if (name.length === 0)
            return
        const row = root.importWorkflow.createActorChoiceForCurrentDraft(root.txRoot.draft, name)
        if (row && row.id !== undefined && String(row.id).length > 0) {
            root.rememberLocalActorRow(row)
            root.importWorkflow.selectCurrentActorChoice(root.txRoot.draft, row)
            root.txRoot.draft.refresh()
        }
    }

    function selectActorChoice(row) {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        if (row && row.id && String(row.id).length > 0) {
            root.rememberLocalActorRow(row)
            root.importWorkflow.selectCurrentActorChoice(root.txRoot.draft, row)
            root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
            root.txRoot.draft.transactions.setContractSelected(root.txRoot.draft.currentIndex, false)
            root.txRoot.draft.refresh()
            return
        }
        root.txRoot.draft.transactions.setActorId(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setActorText(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setActorSelected(root.txRoot.draft.currentIndex, false)
        root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setContractSelected(root.txRoot.draft.currentIndex, false)
        root.txRoot.draft.refresh()
    }

    function commitActorText(value) {
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        root.pendingActorText = nextValue
        root.hasPendingActorEdit = false
    }

    function currentText() {
        return root.pendingActorText
    }

    function syncInputForCurrentTransaction() {
        const tx = root.txRoot && root.txRoot.draft ? root.txRoot.draft.current : null
        const txId = tx && tx.id !== undefined && tx.id !== null ? String(tx.id) : ""
        if (txId === root.lastTxId)
            return
        root.lastTxId = txId
        root.localActorRows = []
        root.hasPendingActorEdit = false
        root.pendingActorText = ""
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.txRoot && root.txRoot.viewState
                    ? root.txRoot.suggestionColor({ confidence: Number(root.txRoot.viewState.actorSuggestionConfidence || 0) })
                    : root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.theme.spacingSmall

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.theme.spacingSmall
                    leftLabel: qsTr("Select Actor")
                    rightLabel: qsTr("Name")
                    leftWeight: 3
                    rightWeight: 2

                    leftContent: Component {
                        Controls.DropdownMenu {
                            id: actorChoiceCombo
                            objectName: "transactionDraftActorChoiceCombo"
                            Layout.fillWidth: true
                            textRole: "display"
                            model: root.actorChoiceModel()
                            currentIndex: root.selectedActorIndex(model)
                            onActivated: function(index) {
                                const row = model[index]
                                root.selectActorChoice(row)
                            }
                        }
                    }

                    rightContent: Component {
                        RowLayout {
                            spacing: root.theme.spacingSmall

                            Controls.TextField {
                                id: actorTextField
                                objectName: "transactionDraftActorTextField"
                                Layout.fillWidth: true
                                placeholderText: ""
                                text: root.currentText()

                                onAccepted: {
                                    root.commitActorText(text)
                                }

                                onEditingFinished: {
                                    root.commitActorText(text)
                                }
                                onTextEdited: {
                                    root.pendingActorText = text
                                    root.hasPendingActorEdit = true
                                }
                                onActiveFocusChanged: if (!activeFocus) root.commitActorText(text)
                            }

                            Controls.SecondaryButton {
                                objectName: "transactionDraftActorAddFromTextButton"
                                text: qsTr("+")
                                Layout.preferredWidth: root.actorControlSize
                                Layout.minimumWidth: root.actorControlSize
                                Layout.maximumWidth: root.actorControlSize
                                Layout.preferredHeight: root.actorControlSize
                                Layout.minimumHeight: root.actorControlSize
                                Layout.maximumHeight: root.actorControlSize
                                textColor: root.theme.textMuted
                                enabled: String(actorTextField.text || "").trim().length > 0 && !root.hasIdenticalActor()
                                onClicked: root.addActorFromText()
                            }
                        }
                    }
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.viewState && root.txRoot.viewState.actorSuggestionSummary
                ? String(root.txRoot.viewState.actorSuggestionSummary)
                : qsTr("0% Confidence - No suggestion")
            color: root.txRoot && root.txRoot.viewState
                ? root.txRoot.suggestionColor({ confidence: Number(root.txRoot.viewState.actorSuggestionConfidence || 0) })
                : root.theme.textMuted
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }

    Connections {
        target: root.txRoot ? root.txRoot.draft : null
        function onCurrentIndexChanged() { root.syncInputForCurrentTransaction() }
        function onCurrentChanged() { root.syncInputForCurrentTransaction() }
        function onCountChanged() { root.syncInputForCurrentTransaction() }
    }
}
