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
    readonly property var draftController: root.appContext ? root.appContext.draftController : null

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
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Label { text: qsTr("Actor"); Layout.fillWidth: true }

        Controls.TextField {
            id: actorTextField
            Layout.fillWidth: true
            placeholderText: qsTr("Actor")
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.actorText || "") : ""

            onTextEdited: if (root.txRoot && root.txRoot.draft) {
                root.txRoot.draft.transactions.setActorText(root.txRoot.draft.currentIndex, text)
                root.txRoot.draft.transactions.setActorId(root.txRoot.draft.currentIndex, "")
                root.txRoot.draft.transactions.setNewActorSelected(root.txRoot.draft.currentIndex, true)
            }

            onAccepted: {
                if (!root.txRoot || !root.txRoot.draft || !root.draftController) return
                const rows = root.txRoot.actorChoices || []
                const row = root.draftController.findChoiceRowByText(rows, text)
                if (row && row.id !== undefined) root.draftController.selectCurrentActorChoice(root.txRoot.draft, row)
                else if (root.txRoot.draft) {
                    root.txRoot.draft.transactions.setActorText(root.txRoot.draft.currentIndex, text)
                    root.txRoot.draft.transactions.setNewActorSelected(root.txRoot.draft.currentIndex, true)
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.newActorSelected && (!root.txRoot.draft.current.actorText || root.txRoot.draft.current.actorText.length === 0)
                ? qsTr("New Actor")
                : (root.txRoot.actorTopSuggestion().label
                    ? qsTr("Confidence: %1% — %2").arg(root.txRoot.suggestionConfidencePercent(root.txRoot.actorTopSuggestion())).arg(root.txRoot.actorTopSuggestion().label)
                    : qsTr("No actor suggestion"))) : qsTr("No actor suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.actorTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }

    }
}
