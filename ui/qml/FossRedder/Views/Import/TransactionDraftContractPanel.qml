/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractPanel.qml
 * @brief Selects and updates contract assignment for the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    property var localContractRows: []
    property string pendingTypeText: ""
    property bool hasPendingTypeEdit: false
    property string pendingNameText: ""
    property bool hasPendingNameEdit: false
    property string pendingAllocatableMode: "mixed"
    property string lastTxId: ""
    readonly property real splitColumnSpacing: root.theme.panelPadding + (root.theme.borderWidthThin * 2)
    readonly property real contractMainLeftWeight: 3.2
    readonly property real contractMainRightWeight: 1.8
    readonly property real contractInnerNameWeight: 2
    readonly property real contractInnerTypeWeight: 1.2

    function normalizedText(value) {
        return String(value || "").trim().replace(/\s+/g, " ").toLowerCase()
    }

    function contractChoiceModel() {
        const baseRows = root.txRoot && root.txRoot.contractChoices ? root.txRoot.contractChoices : []
        const rows = (root.localContractRows || []).concat(baseRows || [])
        const model = [{ id: "", display: qsTr("No contract"), name: "", type: "", actorIds: [], propertyIds: [], allocatableMode: "mixed" }]
        const seenIds = {}
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (!row)
                continue
            const id = row.id !== undefined && row.id !== null ? String(row.id) : ""
            if (id.length === 0)
                continue
            if (seenIds[id])
                continue
            seenIds[id] = true
            const name = row.name !== undefined && row.name !== null ? String(row.name) : ""
            const type = row.type !== undefined && row.type !== null ? String(row.type) : ""
            const allocatableMode = row.allocatableMode !== undefined && row.allocatableMode !== null
                ? String(row.allocatableMode)
                : "mixed"
            let display = row.display !== undefined && row.display !== null ? String(row.display) : ""
            if (name.length > 0)
                display = name
            else if (type.length > 0)
                display = type

            model.push({
                id: id,
                display: display,
                name: name,
                type: type,
                actorIds: row.actorIds !== undefined && row.actorIds !== null ? row.actorIds : [],
                propertyIds: row.propertyIds !== undefined && row.propertyIds !== null ? row.propertyIds : [],
                allocatableMode: allocatableMode
            })
        }
        return model
    }

    function selectedContractIndex(model) {
        const contractId = root.txRoot && root.txRoot.draft && root.txRoot.draft.current
            ? String(root.txRoot.draft.current.contractId || "")
            : ""
        for (let i = 0; i < model.length; ++i) {
            if (String(model[i].id || "") === contractId)
                return i
        }
        return 0
    }

    function selectedContractRow() {
        const model = root.contractChoiceModel()
        return model[root.selectedContractIndex(model)] || ({})
    }

    function selectedContractName() {
        return ""
    }

    function nextGeneratedContractNamePlaceholder() {
        const model = root.contractChoiceModel()
        let maxIndex = 0
        for (let i = 0; i < model.length; ++i) {
            const row = model[i]
            if (!row || !row.name)
                continue
            const rawName = String(row.name).trim()
            const match = /^Contract\s+(\d+)$/i.exec(rawName)
            if (!match || match.length < 2)
                continue
            const idx = Number(match[1])
            if (!isNaN(idx) && idx > maxIndex)
                maxIndex = idx
        }
        return qsTr("Contract %1").arg(maxIndex + 1)
    }

    function commitTypeText(value) {
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        root.pendingTypeText = nextValue
        root.hasPendingTypeEdit = false
    }

    function currentTypeText() {
        return root.pendingTypeText
    }

    function commitNameText(value) {
        root.pendingNameText = value !== undefined && value !== null ? String(value) : ""
        root.hasPendingNameEdit = false
    }

    function currentNameText() {
        return root.pendingNameText
    }

    function currentActorId() {
        if (!root.txRoot || !root.txRoot.draft || !root.txRoot.draft.current)
            return ""
        return String(root.txRoot.draft.current.actorId || "")
    }

    function currentPropertyIds() {
        if (!root.txRoot || !root.txRoot.draft || !root.txRoot.draft.current || !root.txRoot.draft.current.propertyIds)
            return []
        return root.txRoot.draft.current.propertyIds
    }

    function listsEqualAsSet(lhs, rhs) {
        const left = (lhs || []).map(function(v) { return String(v || "") }).sort()
        const right = (rhs || []).map(function(v) { return String(v || "") }).sort()
        if (left.length !== right.length)
            return false
        for (let i = 0; i < left.length; ++i) {
            if (left[i] !== right[i])
                return false
        }
        return true
    }

    function hasIdenticalContract() {
        const type = root.currentTypeText()
        if (normalizedText(type).length === 0)
            return false

        const actorId = root.currentActorId()
        const propertyIds = root.currentPropertyIds()
        const name = root.currentNameText()
        const normalizedName = normalizedText(name)
        const choices = root.contractChoiceModel()
        for (let i = 0; i < choices.length; ++i) {
            const row = choices[i]
            if (!row || !row.id || String(row.id).length === 0)
                continue
            if (normalizedText(row.type) !== normalizedText(type))
                continue
            if (!listsEqualAsSet(row.actorIds || [], actorId.length > 0 ? [actorId] : []))
                continue
            if (!listsEqualAsSet(row.propertyIds || [], propertyIds))
                continue
            if (normalizedName.length > 0 && normalizedText(row.name) !== normalizedName)
                continue
            return true
        }
        return false
    }

    function addContractFromFields() {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        const row = root.importWorkflow.createOrSelectContractChoiceForCurrentDraft(root.txRoot.draft,
                                                                                     root.currentNameText(),
                                                                                     root.currentTypeText(),
                                                                                     root.pendingAllocatableMode)
        if (row && row.id !== undefined && String(row.id).length > 0) {
            root.localContractRows = [row].concat(root.localContractRows || [])
            root.importWorkflow.selectCurrentContractChoice(root.txRoot.draft, row)
            root.pendingNameText = String(row.name || row.display || "")
            root.hasPendingNameEdit = false
        }
    }

    function syncInputsForCurrentTransaction() {
        const tx = root.txRoot && root.txRoot.draft ? root.txRoot.draft.current : null
        const txId = tx && tx.id !== undefined && tx.id !== null ? String(tx.id) : ""
        if (txId === root.lastTxId)
            return
        root.lastTxId = txId
        root.hasPendingTypeEdit = false
        root.pendingTypeText = ""
        root.hasPendingNameEdit = false
        root.pendingNameText = ""
        root.pendingAllocatableMode = "mixed"
        root.localContractRows = []
    }

    function currentSelectedContractType() {
        const selected = root.selectedContractRow()
        return selected && selected.type ? String(selected.type) : ""
    }

    function contractSuggestionColor() {
        if (!(root.txRoot && root.txRoot.viewState))
            return root.theme.textMuted
        const confidence = Number(root.txRoot.viewState.contractSuggestionConfidence || 0)
        return root.txRoot.suggestionColor({ confidence: confidence })
    }

    function selectContractChoice(row) {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        if (row && row.id && String(row.id).length > 0) {
            root.importWorkflow.selectCurrentContractChoice(root.txRoot.draft, row)
        } else {
            root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
            root.txRoot.draft.transactions.setContractSelected(root.txRoot.draft.currentIndex, false)
            root.txRoot.draft.refresh()
        }
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
                border.color: root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.theme.spacingSmall

                Label {
                    text: qsTr("Contract")
                    Layout.fillWidth: true
                    Layout.bottomMargin: -root.theme.spacingSmall
                }

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.splitColumnSpacing
                    leftLabel: ""
                    rightLabel: ""
                    leftWeight: root.contractMainLeftWeight
                    rightWeight: root.contractMainRightWeight

                    leftContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall
                                Label {
                                    text: qsTr("Name")
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerNameWeight
                                }
                                Label {
                                    text: qsTr("Type")
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerTypeWeight
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall
                                Controls.TextField {
                                    objectName: "transactionDraftContractNameField"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerNameWeight
                                    placeholderText: root.nextGeneratedContractNamePlaceholder()
                                    text: root.currentNameText()
                                    onTextEdited: {
                                        root.pendingNameText = text
                                        root.hasPendingNameEdit = true
                                    }
                                    onEditingFinished: root.commitNameText(text)
                                    onAccepted: root.commitNameText(text)
                                    onActiveFocusChanged: if (!activeFocus) root.commitNameText(text)
                                }
                                Controls.TextField {
                                    objectName: "transactionDraftContractTypeField"
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: root.contractInnerTypeWeight
                                    text: root.currentTypeText()
                                    onTextEdited: {
                                        root.pendingTypeText = text
                                        root.hasPendingTypeEdit = true
                                    }
                                    onEditingFinished: root.commitTypeText(text)
                                    onAccepted: root.commitTypeText(text)
                                    onActiveFocusChanged: if (!activeFocus) root.commitTypeText(text)
                                }
                            }
                        }
                    }

                    rightContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall
                            Label { text: qsTr("Allocatable"); Layout.fillWidth: true }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingSmall

                                Controls.DropdownMenu {
                                    id: contractAllocatableModeCombo
                                    objectName: "transactionDraftContractAllocatableModeCombo"
                                    Layout.fillWidth: true
                                    textRole: "label"
                                    model: [
                                        { label: qsTr("Mixed"), value: "mixed" },
                                        { label: qsTr("All allocatable"), value: "allocatable" },
                                        { label: qsTr("Never allocatable"), value: "non-allocatable" }
                                    ]
                                    currentIndex: {
                                        if (root.pendingAllocatableMode === "allocatable")
                                            return 1
                                        if (root.pendingAllocatableMode === "non-allocatable")
                                            return 2
                                        return 0
                                    }
                                    onActivated: function(index) {
                                        const row = model[index]
                                        root.pendingAllocatableMode = row && row.value ? String(row.value) : "mixed"
                                    }
                                }

                                Controls.SecondaryButton {
                                    objectName: "transactionDraftContractAddButton"
                                    text: qsTr("+")
                                    Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                                    Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                                    Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                    Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                    Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                    textColor: root.theme.textMuted
                                    enabled: !root.hasIdenticalContract() && root.currentTypeText().trim().length > 0
                                    onClicked: root.addContractFromFields()
                                }
                            }
                        }
                    }
                }

                TransactionDraftFieldRow {
                    theme: root.theme
                    Layout.fillWidth: true
                    columnSpacing: root.splitColumnSpacing
                    leftLabel: ""
                    rightLabel: ""
                    leftWeight: root.contractMainLeftWeight
                    rightWeight: root.contractMainRightWeight

                    leftContent: Component {
                        ColumnLayout {
                            spacing: root.theme.spacingSmall

                            Controls.Panel {
                                Layout.fillWidth: true
                                background: Rectangle {
                                    radius: root.theme.radius
                                    color: root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.contractSuggestionColor()
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: root.theme.spacingSmall

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: root.theme.spacingSmall

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: root.contractInnerNameWeight
                                            spacing: root.theme.spacingSmall
                                            Label { text: qsTr("Select Contract"); Layout.fillWidth: true }
                                            Controls.DropdownMenu {
                                                id: contractChoiceCombo
                                                objectName: "transactionDraftContractChoiceCombo"
                                                Layout.fillWidth: true
                                                textRole: "display"
                                                model: root.contractChoiceModel()
                                                currentIndex: root.selectedContractIndex(model)
                                                onActivated: function(index) {
                                                    const row = model[index]
                                                    root.selectContractChoice(row)
                                                }
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: root.contractInnerTypeWeight
                                            spacing: root.theme.spacingSmall
                                            Label { text: qsTr("Type"); Layout.fillWidth: true }
                                            Controls.TextField {
                                                objectName: "transactionDraftContractSelectedTypeField"
                                                Layout.fillWidth: true
                                                readOnly: true
                                                text: root.currentSelectedContractType()
                                                color: root.theme.textMuted
                                                background: Rectangle {
                                                    radius: root.theme.radius
                                                    color: root.theme.surface
                                                    border.width: 1
                                                    border.color: root.theme.borderSoft
                                                    opacity: 0.7
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Label {
                                text: root.txRoot && root.txRoot.viewState && root.txRoot.viewState.contractSuggestionSummary
                                      ? String(root.txRoot.viewState.contractSuggestionSummary)
                                      : qsTr("0% Confidence - No suggestion")
                                color: root.contractSuggestionColor()
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.theme.spacingSmall
                            }
                        }
                    }

                    rightContent: Component { Views.TransactionDraftContractAllocatablePanel { txRoot: root.txRoot; theme: root.theme } }
                }

                Views.TransactionDraftContractActorPanel {
                    txRoot: root.txRoot
                    appContext: root.appContext
                    theme: root.theme
                    embedded: true
                }

                Views.TransactionDraftContractPropertyPanel {
                    txRoot: root.txRoot
                    appContext: root.appContext
                    theme: root.theme
                    embedded: true
                }

            }

        }

    }

    Connections {
        target: root.txRoot ? root.txRoot.draft : null
        function onCurrentIndexChanged() { root.syncInputsForCurrentTransaction() }
        function onCurrentChanged() { root.syncInputsForCurrentTransaction() }
        function onCountChanged() { root.syncInputsForCurrentTransaction() }
    }
}
