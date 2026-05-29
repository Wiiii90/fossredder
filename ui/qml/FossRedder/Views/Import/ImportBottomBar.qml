/**
 * @file ui/qml/FossRedder/Views/Import/ImportBottomBar.qml
 * @brief Provides import overview bottom actions.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var importState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "importPreviousDraftButton"
            enabled: root.importState.hasDraftNavigation
            onClicked: root.importState.openPreviousDraft()
        }

        Item { Layout.fillWidth: true }

        Controls.DangerButton {
            objectName: "importClearButton"
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importState.canClear
            enabled: root.importState.canClear
            onClicked: root.importState.resetStatus()
        }

        Controls.DangerButton {
            objectName: "importCancelButton"
            text: qsTr("Cancel")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importState.canCancel
            enabled: root.importState.canCancel
            onClicked: root.importState.cancelImport()
        }

        Controls.DangerButton {
            objectName: "importCancelAllButton"
            text: qsTr("Cancel all")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importState.canCancelAll
            enabled: root.importState.canCancelAll
            onClicked: root.importState.cancelAllImports()
        }

        Controls.SuccessButton {
            objectName: "importPauseButton"
            text: root.importState.pauseText
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.importState.canPause
            enabled: root.importState.canPause
            onClicked: root.importState.togglePause()
        }

        Controls.SuccessButton {
            objectName: "importStartButton"
            text: qsTr("Start")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: !root.importState.importRunning
            enabled: root.importState.canStart
            onClicked: root.importState.startImport()
        }

        Item { Layout.fillWidth: true }

        Controls.NextPageButton {
            objectName: "importNextDraftButton"
            enabled: root.importState.hasDraftNavigation
            onClicked: root.importState.openNextDraft()
        }
    }
}
