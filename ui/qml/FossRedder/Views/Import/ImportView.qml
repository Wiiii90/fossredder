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
    readonly property var actions: root.appContext ? root.appContext.actions : null
    readonly property var status: root.appContext ? root.appContext.status : null
    Component.onCompleted: {
        Qt.callLater(function() {
            root.importPageActivated = true
            root.updateContentIndex()
        })
    }
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    anchors.margins: root.theme.spacingMedium

    property bool hasImportController: root.importController !== null

    property bool importPageActivated: false

    function updateContentIndex() {
        contentStack.currentIndex = (root.hasImportController && root.importController && root.importController.draft) ? 1 : 0
    }


    StackLayout {
        id: contentStack
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
            root.updateContentIndex()
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

                    Controls.Button {
                        text: qsTr("Reset")
                        visible: root.hasImportController && !root.importController.isRunning
                        enabled: root.hasImportController && !root.importController.isRunning
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        bordered: true
                        onClicked: { if (root.hasImportController) root.importController.resetStatus() }
                    }

                    Controls.Button {
                        text: qsTr("Cancel")
                        visible: root.hasImportController && root.importController.isRunning
                        enabled: root.hasImportController && root.importController.isRunning
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        bordered: true
                        onClicked: if (root.hasImportController) root.importController.cancelImport()
                    }

                    Controls.Button {
                        text: qsTr("Pause")
                        visible: root.hasImportController && root.importController.isRunning
                        enabled: root.hasImportController && root.importController.isRunning
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        bordered: true
                    }

                    Controls.Button {
                        text: qsTr("Cancel all")
                        visible: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        enabled: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        bordered: true
                        onClicked: if (root.hasImportController) root.importController.cancelAllImports()
                    }

                    Item { Layout.fillWidth: true }

                    Controls.Button {
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

                Controls.Button {
                    text: "⟪"
                    enabled: !!stmtView.draft && root.hasImportController && root.importController.hasPrevDraft
                    Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                    bordered: true
                    onClicked: if (root.hasImportController) root.importController.openPrevDraft()
                }

                Controls.Button {
                    text: "◀"
                    enabled: !!stmtView.draft && stmtView.draft.currentIndex > 0
                    Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                    bordered: true
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

                Controls.Button {
                    text: "▶"
                    enabled: !!stmtView.draft && stmtView.draft.currentIndex < (stmtView.draft.count - 1)
                    Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                    bordered: true
                    onClicked: {
                        stmtView.draft.next()
                        stmtView.persistDraftSnapshot()
                    }
                }

                Controls.Button {
                    text: "⟫"
                    enabled: !!stmtView.draft && root.hasImportController && root.importController.hasNextDraft
                    Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                    bordered: true
                    onClicked: if (root.hasImportController) root.importController.openNextDraft()
                }
            }
        }
    }
}
