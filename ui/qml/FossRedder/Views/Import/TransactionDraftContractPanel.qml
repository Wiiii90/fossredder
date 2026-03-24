import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: Theme.spacingSmall

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        TransactionDraftFieldRow {
            Layout.fillWidth: true
            leftLabel: qsTr("Type")
            rightLabel: qsTr("Contract")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    text: root.txRoot && root.txRoot.viewState ? (root.txRoot.viewState.contractSeedText || (root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.type || "") : "")) : ""
                    onTextEdited: if (root.txRoot && root.txRoot.draft) {
                        root.txRoot.draft.transactions.setType(root.txRoot.draft.currentIndex, text)
                        root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
                        root.txRoot.draft.transactions.setNewContractSelected(root.txRoot.draft.currentIndex, true)
                    }
                }
            }

            rightContent: Component {
                Controls.ComboBox {
                    editable: true
                    textRole: "display"
                    model: root.txRoot && root.txRoot.viewState ? (root.txRoot.viewState.contractChoices || []) : []
                    currentIndex: root.txRoot && root.txRoot.viewState ? root.txRoot.viewState.contractCurrentIndex : -1
                    editText: root.txRoot && root.txRoot.viewState ? (root.txRoot.viewState.contractDisplayText || "") : ""
                    onActivated: {
                        if (!root.txRoot || !root.txRoot.draft || !draftController) return
                        var rows = root.txRoot.viewState && root.txRoot.viewState.contractChoices ? root.txRoot.viewState.contractChoices : []
                        if (currentIndex >= 0 && currentIndex < rows.length) draftController.selectCurrentContractChoice(root.txRoot.draft, rows[currentIndex])
                    }

                    onEditTextChanged: if (activeFocus && root.txRoot && root.txRoot.draft) {
                        root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
                        root.txRoot.draft.transactions.setNewContractSelected(root.txRoot.draft.currentIndex, true)
                    }

                    onAccepted: {
                        if (!root.txRoot || !root.txRoot.draft || !draftController) return
                        var rows = root.txRoot.viewState && root.txRoot.viewState.contractChoices ? root.txRoot.viewState.contractChoices : []
                        var row = draftController.findChoiceRowByText(rows, editText)
                        if (row && row.id !== undefined) draftController.selectCurrentContractChoice(root.txRoot.draft, row)
                        else if (root.txRoot.draft) {
                            root.txRoot.draft.transactions.setType(root.txRoot.draft.currentIndex, editText)
                            root.txRoot.draft.transactions.setNewContractSelected(root.txRoot.draft.currentIndex, true)
                        }
                    }
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.newContractSelected && (!root.txRoot.draft.current.type || root.txRoot.draft.current.type.length === 0)
                ? qsTr("New Contract")
                : (root.txRoot.contractTopSuggestion().label
                    ? qsTr("Confidence: %1% — %2").arg(root.txRoot.suggestionConfidencePercent(root.txRoot.contractTopSuggestion())).arg(root.txRoot.contractTopSuggestion().label)
                    : qsTr("No contract suggestion"))) : qsTr("No contract suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : Theme.textMuted
            Layout.fillWidth: true
        }
    }
}
