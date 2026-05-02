/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Import/TransactionDraftContractPanel.qml
 * @brief Provides the TransactionDraftContractPanel component.
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
        anchors.fill: parent
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
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }
    }
}
