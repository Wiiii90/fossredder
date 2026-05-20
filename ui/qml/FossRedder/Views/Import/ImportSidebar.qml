/**
 * @file ui/qml/FossRedder/Views/Import/ImportSidebar.qml
 * @brief Provides the ImportSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property var navigation: root.appContext ? root.appContext.navigation : null
    readonly property var session: root.appContext ? root.appContext.session : null

    function selectStatement(statementId) {
        if (!root.session || !statementId || statementId.length === 0) return
        root.session.selectedStatementId = statementId
        root.session.selectedTransactionId = ""
    }

    Connections {
        target: root.session
        function onDataRevisionChanged() {
            if (root.importWorkflow && root.importWorkflow.refreshFromStateSnapshot)
                root.importWorkflow.refreshFromStateSnapshot()
        }
    }

    Component.onCompleted: {
        if (root.importWorkflow && root.importWorkflow.refreshFromStateSnapshot)
            root.importWorkflow.refreshFromStateSnapshot()
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
            model: root.importWorkflow ? root.importWorkflow.runs : null
            selectedLogId: root.importWorkflow && root.importWorkflow.draft
                           ? root.importWorkflow.draft.draftId
                           : ""
            onRunClicked: function(index, logId, draftAttached, statementId, draftId) {
                if (!root.importWorkflow) return
                if (draftAttached) {
                    const targetDraftId = (draftId && draftId.length > 0) ? draftId : logId
                    const currentDraftId = (root.importWorkflow.draft && root.importWorkflow.draft.draftId)
                        ? root.importWorkflow.draft.draftId
                        : ""
                    root.importWorkflow.activateRunAt(index)
                    if (targetDraftId !== currentDraftId)
                        root.importWorkflow.openPersistedDraft(targetDraftId)
                    if (root.navigation)
                        root.navigation.setSectionValue(4)
                    return
                }
                if (!statementId || statementId.length === 0) return
                root.selectStatement(statementId)
                if (root.navigation) {
                    root.navigation.setSectionValue(3)
                    try { root.navigation.setBookingViewValue(0) } catch(e) {}
                }
                Qt.callLater(function() {
                    root.selectStatement(statementId)
                })
            }

            onDeleteClicked: function(index, draftAttached, draftId) {
                if (!root.importWorkflow || !root.importWorkflow.runs) return
                if (draftAttached && root.importWorkflow && root.importWorkflow.clearPersistedStatementDraft)
                    root.importWorkflow.clearPersistedStatementDraft(draftId)
                if (draftAttached) root.importWorkflow.clearDraft()
                root.importWorkflow.removeRunAt(index)
            }
        }
    }
}
