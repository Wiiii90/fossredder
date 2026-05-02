/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Import/ImportProgressBar.qml
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
    required property var importController
    required property bool hasImportController

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        Layout.fillWidth: true
        visible: true
        value: root.hasImportController ? root.importController.progress : 0
    }

    Label {
        Layout.fillWidth: true
        text: root.hasImportController
              ? (root.importController.error && root.importController.error.length > 0
                  ? root.importController.error
                  : (root.importController.phase && root.importController.phase.length > 0
                      ? root.importController.phase
                      : qsTr("Ready")))
              : qsTr("Ready")
        color: root.hasImportController && root.importController.error && root.importController.error.length > 0 ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
