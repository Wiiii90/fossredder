/**
 * @file ui/qml/FossRedder/Views/Export/ExportSidebar.qml
 * @brief Provides the Export sidebar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var exportState
    required property var theme

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
            model: root.exportState.runs
            onRunClicked: function(index, logId, draftAttached, statementId) {
                root.exportState.openRunLocationAt(index)
            }
            onDeleteClicked: function(index, draftAttached, draftId) {
                root.exportState.removeRunAt(index)
            }
        }
    }

    Component.onCompleted: root.exportState.refreshRuns()
}
