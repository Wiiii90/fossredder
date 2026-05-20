/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractPanel.qml
 * @brief Selects and updates contract assignment for the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    property string pendingTypeText: ""
    property bool hasPendingTypeEdit: false

    function selectedContractName() {
        if (!root.txRoot || !root.txRoot.draft || !root.txRoot.draft.current) return ""
        if (!root.txRoot.draft.current.contractId || !root.session || !root.session.contracts) return ""
        const index = root.session.contracts.findRowById(root.txRoot.draft.current.contractId)
        if (index >= 0) {
            const row = root.session.contracts.get(index)
            if (row && row.name) return row.name
        }
        return ""
    }

    function commitTypeText(value) {
        if (!root.txRoot || !root.txRoot.draft)
            return
        const tx = root.txRoot.draft.current
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = tx && tx.type !== undefined && tx.type !== null ? String(tx.type) : ""
        if (nextValue !== currentValue) {
            root.txRoot.draft.transactions.setType(root.txRoot.draft.currentIndex, nextValue)
            root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
            root.txRoot.draft.transactions.setNewContractSelected(root.txRoot.draft.currentIndex, true)
        }
        if (root.txRoot && root.txRoot.refreshDerivedState)
            root.txRoot.refreshDerivedState()
    }

    function currentTypeText() {
        if (root.hasPendingTypeEdit)
            return root.pendingTypeText
        if (root.txRoot && root.txRoot.draft && root.txRoot.draft.current && root.txRoot.draft.current.type !== undefined && root.txRoot.draft.current.type !== null)
            return String(root.txRoot.draft.current.type)
        return ""
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        TransactionDraftFieldRow {
            theme: root.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Type")
            rightLabel: qsTr("Contract")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.type || "") : ""
                    onTextEdited: {
                        root.pendingTypeText = text
                        root.hasPendingTypeEdit = true
                    }
                    onEditingFinished: root.commitTypeText(text)
                    onAccepted: root.commitTypeText(text)
                    onActiveFocusChanged: if (!activeFocus) root.commitTypeText(text)
                }
            }

            rightContent: Component {
                Controls.TextField {
                    Layout.fillWidth: true
                    readOnly: true
                    placeholderText: qsTr("Contract")
                    text: root.selectedContractName()
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.suggestionConfidencePercent(root.txRoot.contractTopSuggestion()) > 0
                ? qsTr("Confidence: %1% — %2").arg(root.txRoot.suggestionConfidencePercent(root.txRoot.contractTopSuggestion())).arg(root.txRoot.contractTopSuggestion().label)
                : qsTr("No contract suggestion")) : qsTr("No contract suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }
    }

    Connections {
        target: root.txRoot ? root.txRoot.draft : null
        function onCurrentIndexChanged() { root.hasPendingTypeEdit = false; root.pendingTypeText = "" }
        function onCurrentChanged() { root.hasPendingTypeEdit = false; root.pendingTypeText = "" }
        function onCountChanged() { root.hasPendingTypeEdit = false; root.pendingTypeText = "" }
    }
}
