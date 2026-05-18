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

                    Controls.SecondaryButton {
                        objectName: "importClearButton"
                        text: qsTr("Clear")
                        visible: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        onClicked: { if (root.hasImportWorkflow) root.importWorkflow.resetStatus() }
                    }

                    Controls.SecondaryButton {
                        objectName: "importCancelButton"
                        text: qsTr("Cancel")
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.cancelImport()
                    }

                    Controls.SecondaryButton {
                        objectName: "importPauseButton"
                        text: qsTr("Pause")
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning
                    }

                    Item { Layout.fillWidth: true }

                    Controls.SecondaryButton {
                        objectName: "importCancelAllButton"
                        text: qsTr("Cancel all")
                        visible: root.hasImportWorkflow && root.importWorkflow.isRunning && root.importWorkflow.queuedCount > 0
                        enabled: root.hasImportWorkflow && root.importWorkflow.isRunning && root.importWorkflow.queuedCount > 0
                        onClicked: if (root.hasImportWorkflow) root.importWorkflow.cancelAllImports()
                    }

                    Controls.SuccessButton {
                        objectName: "importStartButton"
                        text: qsTr("Start")
                        visible: root.hasImportWorkflow && !root.importWorkflow.isRunning
                        enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning && ((root.importWorkflow.selectedFile && root.importWorkflow.selectedFile.length > 0) || root.importWorkflow.queuedCount > 0)
                        onClicked: { if (root.hasImportWorkflow) root.importWorkflow.startStatementImport() }
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
                    onClicked: if (root.hasImportWorkflow) root.importWorkflow.openPrevDraft()
                }

                Controls.PrevButton {
                    objectName: "statementDraftPrevTransactionButton"
                    enabled: !!stmtView.draft && stmtView.draft.currentIndex > 0
                    onClicked: {
                        stmtView.draft.prev()
                        stmtView.persistDraftSnapshot()
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
                        stmtView.draft.next()
                        stmtView.persistDraftSnapshot()
                    }
                }

                Controls.NextPageButton {
                    objectName: "statementDraftNextPageButton"
                    enabled: !!stmtView.draft && root.hasImportWorkflow && root.importWorkflow.hasNextDraft
                    onClicked: if (root.hasImportWorkflow) root.importWorkflow.openNextDraft()
                }
            }
        }
    }
}
