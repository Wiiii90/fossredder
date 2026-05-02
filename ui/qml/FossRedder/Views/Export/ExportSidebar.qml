/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Export/ExportSidebar.qml
 * @brief Provides the ExportSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var exportController: root.appContext ? root.appContext.exportController : null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Components.RunLogList {
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.exportController ? root.exportController.runs : null
            onRunClicked: function(index, logId, draftAttached, statementId) {
                if (!root.exportController) return
                root.exportController.openRunLocationAt(index)
            }
            onDeleteClicked: function(index, draftAttached, draftId) {
                if (!root.exportController || !root.exportController.runs) return
                root.exportController.removeRunAt(index)
            }
        }
    }
}
