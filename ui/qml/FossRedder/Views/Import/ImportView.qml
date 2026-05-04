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
    readonly property var importController: root.appContext ? root.appContext.importController : null
    readonly property var settingsController: root.appContext ? root.appContext.settingsController : null
    readonly property var actions: root.appContext ? root.appContext.actions : null
    readonly property var status: root.appContext ? root.appContext.status : null

    function ensureDefaultImportSelection() {
        if (!root.hasImportController || !root.settingsController || root.importController.isRunning)
            return
        if ((root.importController.selectedFile && root.importController.selectedFile.length > 0) || root.importController.queuedCount > 0)
            return
        if (root.settingsController.importDefaultPath && root.settingsController.importDefaultPath.length > 0)
            root.importController.selectedFile = root.settingsController.importDefaultPath
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

    property bool hasImportController: root.importController !== null

    property bool importPageActivated: false

    function updateContentIndex() {
        contentStack.currentIndex = (root.hasImportController && root.importController && root.importController.draft) ? 1 : 0
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
        target: root.hasImportController ? root.importController : null

        function onStateChanged() {
            root.ensureDefaultImportSelection()
            root.updateContentIndex()
        }
    }

    Connections {
        target: root.settingsController
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
                            importController: root.importController
                            actions: root.actions
                            status: root.status
                        }

                    }
                }

                ImportProgressBar {
                    Layout.fillWidth: true
                    theme: root.theme
                    importController: root.importController
                    hasImportController: root.hasImportController
                }

                Components.BottomBar {
                    Layout.fillWidth: true
                    theme: root.theme

                    Controls.SecondaryButton {
                        objectName: "importClearButton"
                        text: qsTr("Clear")
                        visible: root.hasImportController && !root.importController.isRunning
                        enabled: root.hasImportController && !root.importController.isRunning
                        onClicked: { if (root.hasImportController) root.importController.resetStatus() }
                    }

                    Controls.SecondaryButton {
                        objectName: "importCancelButton"
                        text: qsTr("Cancel")
                        visible: root.hasImportController && root.importController.isRunning
                        enabled: root.hasImportController && root.importController.isRunning
                        onClicked: if (root.hasImportController) root.importController.cancelImport()
                    }

                    Controls.SecondaryButton {
                    objectName: "importPauseButton"
                        text: qsTr("Pause")
                        visible: root.hasImportController && root.importController.isRunning
                        enabled: root.hasImportController && root.importController.isRunning
                    }

                    Item { Layout.fillWidth: true }

                    Controls.SecondaryButton {
                        objectName: "importCancelAllButton"
                        text: qsTr("Cancel all")
                        visible: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        enabled: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        onClicked: if (root.hasImportController) root.importController.cancelAllImports()
                    }

                    Controls.SuccessButton {
                        objectName: "importStartButton"
                        text: qsTr("Start")
                        visible: root.hasImportController && !root.importController.isRunning
                        enabled: root.hasImportController && !root.importController.isRunning && ((root.importController.selectedFile && root.importController.selectedFile.length > 0) || root.importController.queuedCount > 0)
                        onClicked: { if (root.hasImportController) root.importController.startStatementImport() }
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
                draft: (root.hasImportController ? root.importController.draft : null)
            }

            Components.BottomBar {
                id: draftBottomBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                theme: root.theme

                Controls.PrevPageButton {
                    objectName: "statementDraftPrevPageButton"
                    enabled: !!stmtView.draft && root.hasImportController && root.importController.hasPrevDraft
                    onClicked: if (root.hasImportController) root.importController.openPrevDraft()
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
                    enabled: !!stmtView.draft && root.hasImportController && root.importController.hasNextDraft
                    onClicked: if (root.hasImportController) root.importController.openNextDraft()
                }
            }
        }
    }
}
