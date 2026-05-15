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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Components.RunLogList {
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.importWorkflow ? root.importWorkflow.runs : null
            onRunClicked: function(index, logId, draftAttached, statementId) {
                if (!root.importWorkflow) return
                if (draftAttached) {
                    root.importWorkflow.activateRunAt(index)
                    root.importWorkflow.openPersistedDraft(logId)
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
