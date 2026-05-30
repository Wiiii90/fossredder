/**
 * @file ui/qml/FossRedder/Views/Export/ExportBottomBar.qml
 * @brief Provides the Export action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var exportState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.DangerButton {
            objectName: "exportClearButton"
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportState.showClear
            enabled: root.exportState.showClear
            onClicked: root.exportState.clearForm()
        }

        Controls.DangerButton {
            objectName: "exportCancelButton"
            text: qsTr("Cancel")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportState.showCancel
            enabled: root.exportState.showCancel
            onClicked: root.exportState.cancelExport()
        }

        Item { Layout.fillWidth: true }

        Controls.SuccessButton {
            objectName: "exportTogglePauseButton"
            text: root.exportState.pauseText
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportState.showPause
            enabled: root.exportState.showPause
            onClicked: root.exportState.togglePause()
        }

        Controls.SuccessButton {
            objectName: "exportStartButton"
            text: qsTr("Start")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            visible: root.exportState.showClear
            enabled: root.exportState.canStart
            onClicked: root.exportState.startExport()
        }
    }
}
