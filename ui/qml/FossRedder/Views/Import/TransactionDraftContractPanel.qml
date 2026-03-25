import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    function selectedContractName() {
        if (!root.txRoot || !root.txRoot.draft || !root.txRoot.draft.current) return ""
        if (!root.txRoot.draft.current.contractId || !uiData || !uiData.contracts) return ""
        var index = uiData.contracts.findRowById(root.txRoot.draft.current.contractId)
        if (index >= 0) {
            var row = uiData.contracts.get(index)
            if (row && row.name) return row.name
        }
        return ""
    }

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
                    text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.type || "") : ""
                    onTextEdited: if (root.txRoot && root.txRoot.draft) {
                        root.txRoot.draft.transactions.setType(root.txRoot.draft.currentIndex, text)
                        root.txRoot.draft.transactions.setContractId(root.txRoot.draft.currentIndex, "")
                        root.txRoot.draft.transactions.setNewContractSelected(root.txRoot.draft.currentIndex, true)
                    }
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
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : Theme.textMuted
            Layout.fillWidth: true
        }
    }
}
