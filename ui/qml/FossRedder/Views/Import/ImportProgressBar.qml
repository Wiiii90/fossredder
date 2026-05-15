/**
 * @file ui/qml/FossRedder/Views/Import/ImportProgressBar.qml
 * @brief Provides the ImportProgressBar component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var importWorkflow
    required property bool hasImportWorkflow

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        objectName: "importProgressBar"
        Layout.fillWidth: true
        visible: true
        value: root.hasImportWorkflow && root.importWorkflow && typeof root.importWorkflow.progress === "number"
            ? root.importWorkflow.progress
            : 0
    }

    Label {
        Layout.fillWidth: true
        text: root.hasImportWorkflow
              ? (root.importWorkflow.error && root.importWorkflow.error.length > 0
                  ? root.importWorkflow.error
                  : (root.importWorkflow.phase && root.importWorkflow.phase.length > 0
                      ? root.importWorkflow.phase
                      : qsTr("Ready")))
              : qsTr("Ready")
        color: root.hasImportWorkflow && root.importWorkflow.error && root.importWorkflow.error.length > 0 ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
