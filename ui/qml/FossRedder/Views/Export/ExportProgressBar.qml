/**
 * @file ui/qml/FossRedder/Views/Export/ExportProgressBar.qml
 * @brief Provides the ExportProgressBar component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var exportWorkflow
    required property bool hasExportCtrl
    required property string readyText

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        objectName: "exportProgressBar"
        Layout.fillWidth: true
        value: root.hasExportCtrl && root.exportWorkflow.currentMode !== 0 ? root.exportWorkflow.progress : 0
    }

    Label {
        objectName: "exportProgressStatusLabel"
        Layout.fillWidth: true
        text: root.hasExportCtrl
              ? (root.exportWorkflow.error && root.exportWorkflow.error.length > 0
                  ? root.exportWorkflow.error
                  : (root.exportWorkflow.phase && root.exportWorkflow.phase.length > 0
                      ? root.exportWorkflow.phase
                      : root.readyText))
              : root.readyText
        color: root.hasExportCtrl && root.exportWorkflow.error && root.exportWorkflow.error.length > 0 ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
