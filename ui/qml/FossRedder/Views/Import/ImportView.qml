import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
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
    readonly property string automaticSettingsText: qsTr("Settings are currently managed automatically.")
    readonly property string importCanceledStatusText: qsTr("Import canceled")
    readonly property string importFinishedStatusText: qsTr("Import finished")
    readonly property string importFailedStatusText: qsTr("Import failed")
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    anchors.margins: root.theme.spacingMedium

    property bool hasImportController: root.importController !== null

    property bool showAdvanced: false
    property bool showPoppler: false
    property bool showTesseract: false
    property bool showParser: false

    property var pendingFiles: []
    property bool importPageActivated: false

    function manualPathText() {
        return contentStack.manualPathField ? contentStack.manualPathField.text : ""
    }

    function updateContentIndex() {
        contentStack.currentIndex = (root.hasImportController && root.importController && root.importController.draft) ? 1 : 0
    }

    function commitImportFiles(paths, updatePathField) {
        if (!paths || paths.length === 0) return

        const pdfs = []
        for (let i = 0; i < paths.length; ++i) {
            const p = String(paths[i])
            if (!p || p.length === 0) continue
            if (Constants.FileFormats.isSupportedImportPath(p)) pdfs.push(p)
        }
        if (pdfs.length === 0) return

        if (updatePathField && contentStack.manualPathField) contentStack.manualPathField.text = pdfs[0]
        if (root.hasImportController) root.importController.addFiles(pdfs)
        root.pendingFiles = []
    }

    function addImportFiles(paths) {
        root.commitImportFiles(paths, true)
    }

    StackLayout {
        id: contentStack
        anchors.fill: parent
        currentIndex: 0

        property var manualPathField: null

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
                    contentHeight: content.implicitHeight
                    contentWidth: width

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    ColumnLayout {
                        id: content
                        width: scroll.width
                        spacing: root.theme.spacing

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: root.theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: qsTr("Source"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.ComboBox {
                                    id: sourceKind
                                    model: Constants.FileFormats.supportedImportSourceLabels()
                                    currentIndex: 0
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: qsTr("Strategy"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.ComboBox {
                                    id: strategy
                                    model: Constants.FileFormats.supportedStatementStrategyLabels()
                                    currentIndex: 0
                                }
                            }

                            Controls.CheckBox {
                                text: qsTr("Advanced settings")
                                checked: root.showAdvanced
                                onToggled: root.showAdvanced = checked
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: root.showAdvanced
                                spacing: root.theme.spacingSmall

                                Controls.CheckBox {
                                    text: qsTr("Poppler")
                                    checked: root.showPoppler
                                    onToggled: root.showPoppler = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showPoppler
                                    Layout.leftMargin: root.theme.spacing
                                    Label { text: root.automaticSettingsText; color: root.theme.textMuted; wrapMode: Text.WordWrap }
                                }

                                Controls.CheckBox {
                                    text: qsTr("Tesseract")
                                    checked: root.showTesseract
                                    onToggled: root.showTesseract = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showTesseract
                                    Layout.leftMargin: root.theme.spacing
                                    Label { text: root.automaticSettingsText; color: root.theme.textMuted; wrapMode: Text.WordWrap }
                                }

                                Controls.CheckBox {
                                    text: qsTr("Parser")
                                    checked: root.showParser
                                    onToggled: root.showParser = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showParser
                                    Layout.leftMargin: root.theme.spacing
                                    Label { text: root.automaticSettingsText; color: root.theme.textMuted; wrapMode: Text.WordWrap }
                                }
                            }
                        }

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: root.theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true

                                Controls.TextField {
                                    id: manualPath
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("Enter file path...")
                                    enabled: root.hasImportController && !root.importController.isRunning
                                    Component.onCompleted: contentStack.manualPathField = manualPath
                                    onTextEdited: { root.pendingFiles = [] }
                                }

                                Controls.Button {
                                    text: qsTr("Add")
                                    enabled: root.hasImportController && !root.importController.isRunning && root.manualPathText() && root.manualPathText().trim().length > 0
                                    fillColor: root.theme.surface
                                    textColor: root.theme.textPrimary
                                    onClicked: {
                                        let files = []
                                        if (root.pendingFiles && root.pendingFiles.length > 0) files = root.pendingFiles
                                        else files = [root.manualPathText()]
                                        root.commitImportFiles(files, false)
                                        if (contentStack.manualPathField) contentStack.manualPathField.text = ""
                                        root.pendingFiles = []
                                    }
                                }

                                Controls.Button {
                                    text: qsTr("Browse...")
                                    enabled: root.hasImportController && !root.importController.isRunning
                                    fillColor: root.theme.surface
                                    textColor: root.theme.textPrimary
                                    onClicked: { if (root.actions) root.actions.browseImportPdf() }
                                }
                            }

                            Controls.DropZone {
                                Layout.fillWidth: true
                                enabled: root.hasImportController && !root.importController.isRunning
                                title: qsTr("Drop PDFs here")
                                subtitle: ""
                                allowBrowse: false
                                clickToBrowse: true
                                queuedCount: root.hasImportController ? root.importController.queuedCount : 0
                                files: root.hasImportController ? (root.importController.selectedFile && root.importController.selectedFile.length > 0 ? [root.importController.selectedFile].concat(root.importController.queuedFiles) : root.importController.queuedFiles) : []
                                onBrowseRequested: { if (root.actions) root.actions.browseImportPdf() }
                            }

                            Connections {
                                target: root.actions
                                function onImportFileSelected(path) {
                                    if (!path) return
                                    manualPath.text = path
                                    root.pendingFiles = [path]
                                }
                                function onImportFilesSelected(paths) {
                                    if (!paths || paths.length === 0) return
                                    manualPath.text = paths[0]
                                    root.pendingFiles = paths
                                }
                                function onImportFileDropped(path) {
                                    if (!path) return
                                    root.commitImportFiles([path], false)
                                }
                                function onImportFilesDropped(paths) {
                                    if (!paths || paths.length === 0) return
                                    root.commitImportFiles(paths, false)
                                }
                            }

                            Connections {
                                target: root.hasImportController ? root.importController : null
                                function onImportCanceled() {
                                    if (root.status) root.status.text = root.importCanceledStatusText
                                }
                                function onImportFinished() {
                                    if (root.status) root.status.text = root.importFinishedStatusText
                                }
                                function onImportFailed(error) {
                                    if (root.status)
                                        root.status.text = (error && error.length > 0) ? error : root.importFailedStatusText
                                }
                            }
                        }

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: root.theme.spacingSmall

                            Controls.ProgressBar {
                                Layout.fillWidth: true
                                visible: root.hasImportController && (root.importController.isRunning || root.importController.progress > 0)
                                value: root.hasImportController ? root.importController.progress : 0
                            }

                            Label {
                                Layout.fillWidth: true
                                text: root.hasImportController ? (root.importController.error && root.importController.error.length > 0 ? root.importController.error : root.importController.phase) : ""
                                color: root.hasImportController && root.importController.error && root.importController.error.length > 0 ? root.theme.danger : root.theme.textPrimary
                                wrapMode: Text.WordWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: root.hasImportController ? (root.importController.isRunning && root.importController.pageCount > 0) : false
                                text: root.hasImportController ? qsTr("Page %1/%2").arg(root.importController.currentPage).arg(root.importController.pageCount) : ""
                                color: root.theme.textMuted
                                wrapMode: Text.WordWrap
                            }
                        }

                        Item { Layout.preferredHeight: root.theme.spacingLarge }
                    }
                }

                Components.BottomBar {
                    Layout.fillWidth: true
                    theme: root.theme

                    Controls.Button {
                        text: qsTr("Cancel")
                        visible: root.hasImportController && root.importController.isRunning
                        enabled: root.hasImportController && root.importController.isRunning
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        onClicked: if (root.hasImportController) root.importController.cancelImport()
                    }

                    Controls.Button {
                        text: qsTr("Cancel all")
                        visible: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        enabled: root.hasImportController && root.importController.isRunning && root.importController.queuedCount > 0
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        onClicked: if (root.hasImportController) root.importController.cancelAllImports()
                    }

                    Controls.Button {
                        text: qsTr("Reset")
                        visible: root.hasImportController && !root.importController.isRunning
                        enabled: root.hasImportController && !root.importController.isRunning
                        fillColor: root.theme.surface
                        textColor: root.theme.textPrimary
                        onClicked: { if (root.hasImportController) root.importController.resetStatus() }
                    }

                    Item { Layout.fillWidth: true }

                    Controls.Button {
                        text: qsTr("Start")
                        visible: root.hasImportController && !root.importController.isRunning
                        enabled: root.hasImportController && !root.importController.isRunning && ((root.importController.selectedFile && root.importController.selectedFile.length > 0) || root.importController.queuedCount > 0)
                        onClicked: { if (root.hasImportController) root.importController.startStatementImport() }
                    }

                    BusyIndicator {
                        id: importBusy
                        running: root.hasImportController && root.importController.isRunning
                        visible: importBusy.running
                        width: root.theme.busyIndicatorSize
                        height: root.theme.busyIndicatorSize
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
                    text: qsTr("Previous")
                    enabled: stmtView.draft && stmtView.draft.currentIndex > 0
                    onClicked: {
                        if (stmtView.draft) stmtView.draft.prev()
                        if (stmtView.forceSync) stmtView.forceSync()
                    }
                }

                Controls.Button {
                    text: qsTr("Next")
                    enabled: stmtView.draft && (stmtView.draft.currentIndex < stmtView.draft.count - 1)
                    onClicked: {
                        if (stmtView.draft) stmtView.draft.next()
                        if (stmtView.forceSync) stmtView.forceSync()
                    }
                }

                Item { Layout.fillWidth: true }

                Controls.Button {
                    text: qsTr("Discard")
                    enabled: !!stmtView.draft
                    onClicked: stmtView.discardDraft()
                }

                Controls.Button {
                    text: qsTr("Finalize")
                    enabled: !!stmtView.draft
                    onClicked: stmtView.finalizeDraft()
                }
            }
        }
    }
}
