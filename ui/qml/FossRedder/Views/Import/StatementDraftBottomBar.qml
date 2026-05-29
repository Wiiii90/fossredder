/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftBottomBar.qml
 * @brief Provides statement draft review bottom actions.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var statementState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "statementDraftPrevPageButton"
            enabled: root.statementState.canOpenPreviousDraft
            onClicked: root.statementState.openPreviousDraft()
        }

        Controls.PrevButton {
            objectName: "statementDraftPrevTransactionButton"
            enabled: root.statementState.canOpenPreviousTransaction
            onClicked: root.statementState.openPreviousTransaction()
        }

        Item { Layout.fillWidth: true }

        Controls.ReturnButton {
            objectName: "statementDraftReturnButton"
            enabled: root.statementState.hasDraft
            onClicked: root.statementState.returnToImport()
        }

        Controls.DangerButton {
            objectName: "statementDraftDiscardButton"
            text: qsTr("Discard")
            enabled: root.statementState.hasDraft
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.statementState.discardDraft()
        }

        Controls.SuccessButton {
            objectName: "statementDraftFinalizeButton"
            text: qsTr("Finalize")
            enabled: root.statementState.hasDraft
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.statementState.finalizeDraft()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "statementDraftNextTransactionButton"
            enabled: root.statementState.canOpenNextTransaction
            onClicked: root.statementState.openNextTransaction()
        }

        Controls.NextPageButton {
            objectName: "statementDraftNextPageButton"
            enabled: root.statementState.canOpenNextDraft
            onClicked: root.statementState.openNextDraft()
        }
    }
}
