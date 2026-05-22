/**
 * @file ui/qml/FossRedder/Views/Import/ImportView.qml
 * @brief Composes import workflow pages and bottom actions for statement import runs.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property var settingsViewModel: root.appContext ? root.appContext.settingsViewModel : null
    readonly property var actions: root.appContext ? root.appContext.actions : null
    readonly property var status: root.appContext ? root.appContext.status : null
    readonly property var session: (root.appContext && root.appContext.session) ? root.appContext.session : null

    function ensureDefaultImportSelection() {
        if (!root.hasImportWorkflow || !root.settingsViewModel || root.importWorkflow.isRunning)
            return
        if ((root.importWorkflow.selectedFile && root.importWorkflow.selectedFile.length > 0) || root.importWorkflow.queuedCount > 0)
            return
        if (root.settingsViewModel.importDefaultPath && root.settingsViewModel.importDefaultPath.length > 0)
            root.importWorkflow.selectedFile = root.settingsViewModel.importDefaultPath
    }

    Component.onCompleted: {
        Qt.callLater(function() {
            root.importPageActivated = true
            root.ensureDefaultImportSelection()
            root.updateContentIndex()
        })
    }
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    anchors.margins: root.theme.pageContentMargin

    property bool hasImportWorkflow: root.importWorkflow !== null

    property bool importPageActivated: false

    function hasAnyDraftNavigation() {
        if (!root.hasImportWorkflow)
            return false
        return root.importWorkflow.hasDraftStack
    }

    function updateContentIndex() {
        contentStack.currentIndex = (root.hasImportWorkflow && root.importWorkflow && root.importWorkflow.draft) ? 1 : 0
    }

    StackLayout {
        id: contentStack
        objectName: "importContentStack"
        anchors.fill: parent
        currentIndex: 0

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            active: root.importPageActivated && contentStack.currentIndex === 0
            asynchronous: false
            sourceComponent: importPageComponent
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            active: root.importPageActivated && contentStack.currentIndex === 1
            asynchronous: false
            sourceComponent: statementDraftPageComponent
        }
    }

    Connections {
        target: root.hasImportWorkflow ? root.importWorkflow : null

        function onStateChanged() {
            root.ensureDefaultImportSelection()
            root.updateContentIndex()
        }
    }

    Connections {
        target: root.session
        ignoreUnknownSignals: true
        function onDataRevisionChanged() {
            root.ensureDefaultImportSelection()
            root.updateContentIndex()
        }
    }

    Connections {
        target: root.settingsViewModel
        function onImportDefaultPathChanged() {
            root.ensureDefaultImportSelection()
        }
    }

    Component {
        id: importPageComponent

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: root.theme.spacing

                Flickable {
                    id: scroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentHeight: Math.max(content.implicitHeight, scroll.height)
                    contentWidth: width

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: content
                        width: scroll.width
                        height: Math.max(implicitHeight, scroll.height)
                        spacing: root.theme.spacing

                        ImportForm {
                            Layout.fillWidth: true
                            theme: root.theme
                        }

                        ImportPanel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 260
                            theme: root.theme
                            importWorkflow: root.importWorkflow
                            actions: root.actions
                            status: root.status
                        }

                    }
                }

                ImportProgressBar {
                    Layout.fillWidth: true
                    theme: root.theme
                    importWorkflow: root.importWorkflow
                    hasImportWorkflow: root.hasImportWorkflow
                }

                Components.BottomBar {
                    Layout.fillWidth: true
                    theme: root.theme

                    Controls.PrevPageButton {
                        objectName: "importPreviousDraftButton"
                        enabled: root.hasAnyDraftNavigation()
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.openPrevDraft()
                    }

                    Item { Layout.fillWidth: true }

                    Controls.DangerButton {
                        objectName: "importClearButton"
                        text: qsTr("Clear")
                        Layout.preferredWidth: root.theme.viewActionButtonWidth
                        visible: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        onClicked: { if (root.hasImportWorkflow) root.importWorkflow.resetStatus() }
                    }

                    Controls.DangerButton {
                        objectName: "importCancelButton"
                        text: qsTr("Cancel")
                        Layout.preferredWidth: root.theme.viewActionButtonWidth
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.cancelImport()
                    }

                    Controls.DangerButton {
                        objectName: "importCancelAllButton"
                        text: qsTr("Cancel all")
                        Layout.preferredWidth: root.theme.viewActionButtonWidth
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning && root.importWorkflow.queuedCount > 0
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning && root.importWorkflow.queuedCount > 0
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.cancelAllImports()
                    }


                    Controls.SuccessButton {
                        objectName: "importPauseButton"
                        text: root.hasImportWorkflow && root.importWorkflow.isPaused ? qsTr("Resume") : qsTr("Pause")
                        Layout.preferredWidth: root.theme.viewActionButtonWidth
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.togglePause()
                    }

                    Controls.SuccessButton {
                        objectName: "importStartButton"
                        text: qsTr("Start")
                        Layout.preferredWidth: root.theme.viewActionButtonWidth
                        visible: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning && ((root.importWorkflow.selectedFile && root.importWorkflow.selectedFile.length > 0) || root.importWorkflow.queuedCount > 0)
                        onClicked: { if (root.hasImportWorkflow) root.importWorkflow.startStatementImport() }
                    }

                    Item { Layout.fillWidth: true }

                    Controls.NextPageButton {
                        objectName: "importNextDraftButton"
                        enabled: root.hasAnyDraftNavigation()
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.openNextDraft()
                    }
                }
            }
        }
    }

    Component {
        id: statementDraftPageComponent

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StatementDraftView {
                id: stmtView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: draftBottomBar.top
                anchors.bottomMargin: root.theme.spacingSmall
                appContext: root.appContext
                theme: root.theme
                draft: (root.hasImportWorkflow ? root.importWorkflow.draft : null)
            }

            Components.BottomBar {
                id: draftBottomBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                theme: root.theme

                Controls.PrevPageButton {
                    objectName: "statementDraftPrevPageButton"
                    enabled: !!stmtView.draft && root.hasImportWorkflow && root.importWorkflow.hasPrevDraft
                    onClicked: {
                        if (stmtView && stmtView.persistPendingEdits) stmtView.persistPendingEdits()
                        if (root.hasImportWorkflow) root.importWorkflow.openPrevDraft()
                    }
                }

                Controls.PrevButton {
                    objectName: "statementDraftPrevTransactionButton"
                    enabled: !!stmtView.draft && stmtView.draft.currentIndex > 0
                    onClicked: {
                        if (stmtView && stmtView.commitPendingEdits) stmtView.commitPendingEdits()
                        stmtView.draft.prev()
                    }
                }

                Item { Layout.fillWidth: true }

                Controls.Button {
                    text: "↩"
                    enabled: !!stmtView.draft
                    Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                    bordered: true
                    onClicked: stmtView.returnToImport()
                }

                Controls.DangerButton {
                    text: qsTr("Discard")
                    enabled: !!stmtView.draft
                    Layout.preferredWidth: root.theme.viewActionButtonWidth
                    onClicked: stmtView.discardDraft()
                }

                Controls.SuccessButton {
                    text: qsTr("Finalize")
                    enabled: !!stmtView.draft
                    Layout.preferredWidth: root.theme.viewActionButtonWidth
                    onClicked: stmtView.finalizeDraft()
                }

                Item { Layout.fillWidth: true }

                Controls.NextButton {
                    objectName: "statementDraftNextTransactionButton"
                    enabled: !!stmtView.draft && stmtView.draft.currentIndex < (stmtView.draft.count - 1)
                    onClicked: {
                        if (stmtView && stmtView.commitPendingEdits) stmtView.commitPendingEdits()
                        stmtView.draft.next()
                    }
                }

                Controls.NextPageButton {
                    objectName: "statementDraftNextPageButton"
                    enabled: !!stmtView.draft && root.hasImportWorkflow && root.importWorkflow.hasNextDraft
                    onClicked: {
                        if (stmtView && stmtView.persistPendingEdits) stmtView.persistPendingEdits()
                        if (root.hasImportWorkflow) root.importWorkflow.openNextDraft()
                    }
                }
            }
        }
    }
}
