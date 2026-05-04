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
    required property var exportCtrl
    required property bool hasExportCtrl
    required property string readyText

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        objectName: "exportProgressBar"
        Layout.fillWidth: true
        value: root.hasExportCtrl && root.exportCtrl.currentMode !== 0 ? root.exportCtrl.progress : 0
    }

    Label {
        objectName: "exportProgressStatusLabel"
        Layout.fillWidth: true
        text: root.hasExportCtrl
              ? (root.exportCtrl.error && root.exportCtrl.error.length > 0
                  ? root.exportCtrl.error
                  : (root.exportCtrl.phase && root.exportCtrl.phase.length > 0
                      ? root.exportCtrl.phase
                      : root.readyText))
              : root.readyText
        color: root.hasExportCtrl && root.exportCtrl.error && root.exportCtrl.error.length > 0 ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
