/**
 * @file ui/qml/FossRedder/Views/Export/ExportSidebar.qml
 * @brief Provides the ExportSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var exportWorkflow: root.appContext ? root.appContext.exportWorkflow : null

    Connections {
        target: root.appContext ? root.appContext.session : null
        function onDataRevisionChanged() {
            if (root.exportWorkflow && root.exportWorkflow.refreshFromStateSnapshot)
                root.exportWorkflow.refreshFromStateSnapshot()
        }
    }

    Component.onCompleted: {
        if (root.exportWorkflow && root.exportWorkflow.refreshFromStateSnapshot)
            root.exportWorkflow.refreshFromStateSnapshot()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Components.RunLogList {
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            cardMinHeight: root.theme.viewSidebarRowHeight + root.theme.spacingSmall
            cardRadius: root.theme.viewSidebarRowRadius
            cardPadding: root.theme.spacingSmall
            listTopMargin: 0
            itemSpacing: root.theme.spacingSmall
            baseBorderColor: root.theme.borderSoft
            hoverBorderColor: root.theme.borderSoft
            actionButtonSize: root.theme.viewCompactActionButtonSizeTiny
            actionButtonTopInset: 0
            actionButtonRightInset: 0
            headerTopInset: root.theme.spacingSmall
            model: root.exportWorkflow ? root.exportWorkflow.runs : null
            onRunClicked: function(index, logId, draftAttached, statementId) {
                if (!root.exportWorkflow) return
                root.exportWorkflow.openRunLocationAt(index)
            }
            onDeleteClicked: function(index, draftAttached, draftId) {
                if (!root.exportWorkflow || !root.exportWorkflow.runs) return
                root.exportWorkflow.removeRunAt(index)
            }
        }
    }
}
