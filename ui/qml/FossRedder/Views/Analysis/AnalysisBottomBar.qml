/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisBottomBar.qml
 * @brief Provides the Analysis view action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var analysisState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevButton {
            objectName: "analysisPreviousButton"
            enabled: root.analysisState.hasRows
            onClicked: root.analysisState.navigate(-1)
        }

        Item { Layout.fillWidth: true }

        Controls.SecondaryButton {
            objectName: "analysisToggleWorkspaceButton"
            visible: !root.analysisState.isEdit && root.analysisState.filterEditMode
            text: "⇆"
            Layout.preferredWidth: 48
            onClicked: root.analysisState.toggleFilterWorkspace()
        }

        Controls.DangerButton {
            objectName: "analysisResetButton"
            visible: !root.analysisState.isEdit
            text: qsTr("Reset")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisState.clearFilters()
        }

        Controls.SuccessButton {
            objectName: "analysisCreateButton"
            visible: !root.analysisState.isEdit
            text: qsTr("Create")
            enabled: root.analysisState.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisState.submitCreate()
        }

        Controls.DangerButton {
            objectName: "analysisDeleteButton"
            visible: root.analysisState.isEdit
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisState.deleteCurrent()
        }

        Controls.SuccessButton {
            objectName: "analysisUpdateButton"
            visible: root.analysisState.isEdit
            text: qsTr("Update")
            enabled: root.analysisState.canSubmit
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.analysisState.submitUpdate()
        }

        Item { Layout.fillWidth: true }

        Controls.NextButton {
            objectName: "analysisNextButton"
            enabled: root.analysisState.hasRows
            onClicked: root.analysisState.navigate(1)
        }
    }
}
