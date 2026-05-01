/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Import/ImportSidebar.qml
 * @brief Provides the ImportSidebar component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var importController: root.appContext ? root.appContext.importController : null
    readonly property var draftController: root.appContext ? root.appContext.draftController : null
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
            model: root.importController ? root.importController.runs : null
            onRunClicked: function(index, logId, draftAttached, statementId) {
                if (!root.importController) return
                if (draftAttached) {
                    root.importController.activateRunAt(index)
                    root.importController.openPersistedDraft(logId)
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
                if (!root.importController || !root.importController.runs) return
                if (draftAttached && root.draftController && root.draftController.clearPersistedStatementDraft)
                    root.draftController.clearPersistedStatementDraft(draftId)
                if (draftAttached) root.importController.clearDraft()
                root.importController.removeRunAt(index)
            }
        }
    }
}
