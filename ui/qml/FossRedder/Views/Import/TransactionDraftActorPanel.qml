/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftActorPanel.qml
 * @brief Edits actor text and actor matching for the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property real actorControlSize: root.theme.viewCompactActionButtonSize

    function actorChoiceModel() {
        const rows = root.txRoot && root.txRoot.actorChoices ? root.txRoot.actorChoices : []
        const model = [{ id: "", display: qsTr("No actor") }]
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (!row)
                continue
            const id = row.id !== undefined && row.id !== null ? String(row.id) : ""
            if (id.length === 0)
                continue
            model.push({
                id: id,
                display: row.display !== undefined && row.display !== null ? String(row.display) : ""
            })
        }
        return model
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
        if (row && row.id !== undefined && String(row.id).length > 0)
            root.importWorkflow.selectCurrentActorChoice(root.txRoot.draft, row)
        if (root.txRoot.refreshDerivedState)
            root.txRoot.refreshDerivedState()
    }

    function selectActorChoice(row) {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        if (row && row.id && String(row.id).length > 0) {
            root.importWorkflow.selectCurrentActorChoice(root.txRoot.draft, row)
            return
        }
        root.txRoot.draft.transactions.setActorId(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setActorText(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setActorSelected(root.txRoot.draft.currentIndex, false)
        root.txRoot.draft.refresh()
        if (root.txRoot.refreshDerivedState)
            root.txRoot.refreshDerivedState()
    }

    function commitActorText(value) {
        if (!root.txRoot || !root.txRoot.draft)
            return
        const tx = root.txRoot.draft.current
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = tx && tx.actorText !== undefined && tx.actorText !== null ? String(tx.actorText) : ""
        if (nextValue === currentValue)
            return
        root.txRoot.draft.transactions.setActorText(root.txRoot.draft.currentIndex, nextValue)
        root.txRoot.draft.transactions.setActorId(root.txRoot.draft.currentIndex, "")
        root.txRoot.draft.transactions.setActorSelected(root.txRoot.draft.currentIndex, true)
    }

    function currentText() {
        return actorTextField.text
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.actorTopSuggestion()) : root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label { text: qsTr("Actor"); Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 3
                spacing: root.theme.spacingSmall

                Controls.TextField {
                    id: actorTextField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Actor")
                    text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.actorText || "") : ""

                    onAccepted: {
                        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow) return
                        const rows = root.txRoot.actorChoices || []
                        const row = root.importWorkflow.findChoiceRowByText(rows, text)
                        if (row && row.id !== undefined && String(row.id).length > 0) {
                            root.importWorkflow.selectCurrentActorChoice(root.txRoot.draft, row)
                        } else {
                            root.commitActorText(text)
                        }
                        if (root.txRoot && root.txRoot.refreshDerivedState) root.txRoot.refreshDerivedState()
                    }

                    onEditingFinished: {
                        root.commitActorText(text)
                        if (root.txRoot && root.txRoot.refreshDerivedState) root.txRoot.refreshDerivedState()
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
                    enabled: String(actorTextField.text || "").trim().length > 0
                    onClicked: root.addActorFromText()
                }
            }

            Controls.DropdownMenu {
                id: actorChoiceCombo
                objectName: "transactionDraftActorChoiceCombo"
                Layout.fillWidth: true
                Layout.preferredWidth: 2
                textRole: "display"
                model: root.actorChoiceModel()
                currentIndex: root.selectedActorIndex(model)
                onActivated: function(index) {
                    const row = model[index]
                    root.selectActorChoice(row)
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.actorSelected && (!root.txRoot.draft.current.actorText || root.txRoot.draft.current.actorText.length === 0)
                ? qsTr("New Actor")
                : (root.txRoot.actorTopSuggestion().label
                    ? qsTr("Confidence: %1% — %2").arg(root.txRoot.suggestionConfidencePercent(root.txRoot.actorTopSuggestion())).arg(root.txRoot.actorTopSuggestion().label)
                    : qsTr("No actor suggestion"))) : qsTr("No actor suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.actorTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }

    }
}
