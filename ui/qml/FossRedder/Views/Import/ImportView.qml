/**
 * @file ui/qml/FossRedder/Views/Import/ImportView.qml
 * @brief Composes import workflow pages for statement import runs.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as App
import FossRedder.Views.Import 1.0 as Import
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    App.ImportState {
        id: importState
        importWorkflow: root.appContext.importWorkflow
        settingsViewModel: root.appContext.settingsViewModel
        actions: root.appContext.actions
        navigation: root.appContext.navigation
        status: root.appContext.status
        workspace: root.appContext.workspaceFacade
    }

    Component.onCompleted: Qt.callLater(importState.activatePage)

    StackLayout {
        objectName: "importContentStack"
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        currentIndex: importState.contentIndex

        Import.ImportHomeView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importState: importState
        }

        Import.StatementDraftView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            importWorkflow: root.appContext.importWorkflow
            navigation: root.appContext.navigation
            workspace: root.appContext.workspaceFacade
            draft: importState.currentDraft
        }
    }
}
