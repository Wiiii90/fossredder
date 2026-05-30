/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualBottomBar.qml
 * @brief Provides the Annual view action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var annualState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevButton {
            objectName: "annualPreviousButton"
            enabled: root.annualState.hasRows
            onClicked: root.annualState.navigate(-1)
        }

        Item { Layout.fillWidth: true }

        Controls.SecondaryButton {
            objectName: "annualToggleWorkspaceButton"
            text: "⇆"
            Layout.preferredWidth: root.theme.viewNavigationButtonWidth
            onClicked: root.annualState.toggleWorkspace()
        }

        Controls.DangerButton {
            objectName: "annualClearButton"
            visible: !root.annualState.isEdit
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualState.resetCreateState()
        }

        Controls.SuccessButton {
            objectName: "annualCreateButton"
            visible: !root.annualState.isEdit
            text: qsTr("Create")
            enabled: root.annualState.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualState.submitCreate()
        }

        Controls.DangerButton {
            objectName: "annualDeleteButton"
            visible: root.annualState.isEdit
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualState.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "annualUpdateButton"
            visible: root.annualState.isEdit
            text: qsTr("Update")
            enabled: root.annualState.canSubmit && root.annualState.hasChanges
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.annualState.submitUpdate()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "annualNextButton"
            enabled: root.annualState.hasRows
            onClicked: root.annualState.navigate(1)
        }
    }
}
