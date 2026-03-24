import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import "../../Constants/FileFormats.js" as FileFormats

Item {
    id: root
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
    anchors.margins: Theme.spacingMedium

    property bool hasImportController: typeof importController !== 'undefined'

    property bool showAdvanced: false
    property bool showPoppler: false
    property bool showTesseract: false
    property bool showParser: false

    property var pendingFiles: []
    property bool importPageActivated: false

    function updateContentIndex() {
        contentStack.currentIndex = (hasImportController && importController && importController.draft) ? 1 : 0
    }

    function commitImportFiles(paths, updatePathField) {
        if (!paths || paths.length === 0) return

        var pdfs = []
        for (var i = 0; i < paths.length; ++i) {
            var p = String(paths[i])
            if (!p || p.length === 0) continue
            if (FileFormats.isSupportedImportPath(p)) pdfs.push(p)
        }
        if (pdfs.length === 0) return

        if (updatePathField && manualPath) manualPath.text = pdfs[0]
        if (hasImportController) importController.addFiles(pdfs)
        pendingFiles = []
    }

    function addImportFiles(paths) {
        commitImportFiles(paths, true)
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
        target: hasImportController ? importController : null

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
                spacing: Theme.spacing

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
                        spacing: Theme.spacing

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: qsTr("Source"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.ComboBox {
                                    id: sourceKind
                                    model: FileFormats.supportedImportSourceLabels()
                                    currentIndex: 0
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: qsTr("Strategy"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.ComboBox {
                                    id: strategy
                                    model: FileFormats.supportedStatementStrategyLabels()
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
                                spacing: Theme.spacingSmall

                                Controls.CheckBox {
                                    text: qsTr("Poppler")
                                    checked: root.showPoppler
                                    onToggled: root.showPoppler = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showPoppler
                                    Layout.leftMargin: Theme.spacing
                                    Label { text: root.automaticSettingsText; color: Theme.textMuted; wrapMode: Text.WordWrap }
                                }

                                Controls.CheckBox {
                                    text: qsTr("Tesseract")
                                    checked: root.showTesseract
                                    onToggled: root.showTesseract = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showTesseract
                                    Layout.leftMargin: Theme.spacing
                                    Label { text: root.automaticSettingsText; color: Theme.textMuted; wrapMode: Text.WordWrap }
                                }

                                Controls.CheckBox {
                                    text: qsTr("Parser")
                                    checked: root.showParser
                                    onToggled: root.showParser = checked
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showParser
                                    Layout.leftMargin: Theme.spacing
                                    Label { text: root.automaticSettingsText; color: Theme.textMuted; wrapMode: Text.WordWrap }
                                }
                            }
                        }

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                            RowLayout {
                                Layout.fillWidth: true

                                Controls.TextField {
                                    id: manualPath
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("Enter file path...")
                                    enabled: root.hasImportController && !importController.isRunning
                                    onTextEdited: { root.pendingFiles = [] }
                                }

                                Controls.Button {
                                    text: qsTr("Add")
                                    enabled: root.hasImportController && !importController.isRunning && manualPath.text && manualPath.text.trim().length > 0
                                    fillColor: Theme.surface
                                    textColor: Theme.textPrimary
                                    onClicked: {
                                        var files = []
                                        if (root.pendingFiles && root.pendingFiles.length > 0) files = root.pendingFiles
                                        else files = [manualPath.text]
                                        root.commitImportFiles(files, false)
                                        manualPath.text = ""
                                        root.pendingFiles = []
                                    }
                                }

                                Controls.Button {
                                    text: qsTr("Browse...")
                                    enabled: root.hasImportController && !importController.isRunning
                                    fillColor: Theme.surface
                                    textColor: Theme.textPrimary
                                    onClicked: { if (uiActions) uiActions.browseImportPdf() }
                                }
                            }

                            Controls.DropZone {
                                Layout.fillWidth: true
                                enabled: root.hasImportController && !importController.isRunning
                                title: qsTr("Drop PDFs here")
                                subtitle: ""
                                allowBrowse: false
                                clickToBrowse: true
                                queuedCount: root.hasImportController ? importController.queuedCount : 0
                                files: root.hasImportController ? (importController.selectedFile && importController.selectedFile.length > 0 ? [importController.selectedFile].concat(importController.queuedFiles) : importController.queuedFiles) : []
                                onBrowseRequested: { if (uiActions) uiActions.browseImportPdf() }
                            }

                            Connections {
                                target: uiActions
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
                                target: root.hasImportController ? importController : null
                                function onImportCanceled() {
                                    if (typeof uiStatus !== 'undefined' && uiStatus) uiStatus.text = root.importCanceledStatusText
                                }
                                function onImportFinished() {
                                    if (typeof uiStatus !== 'undefined' && uiStatus) uiStatus.text = root.importFinishedStatusText
                                }
                                function onImportFailed(error) {
                                    if (typeof uiStatus !== 'undefined' && uiStatus)
                                        uiStatus.text = (error && error.length > 0) ? error : root.importFailedStatusText
                                }
                            }
                        }

                        Controls.Panel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                            Controls.ProgressBar {
                                Layout.fillWidth: true
                                visible: root.hasImportController && (importController.isRunning || importController.progress > 0)
                                value: root.hasImportController ? importController.progress : 0
                            }

                            Label {
                                Layout.fillWidth: true
                                text: root.hasImportController ? (importController.error && importController.error.length > 0 ? importController.error : importController.phase) : ""
                                color: root.hasImportController && importController.error && importController.error.length > 0 ? Theme.danger : Theme.textPrimary
                                wrapMode: Text.WordWrap
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: root.hasImportController ? (importController.isRunning && importController.pageCount > 0) : false
                                text: root.hasImportController ? qsTr("Page %1/%2").arg(importController.currentPage).arg(importController.pageCount) : ""
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                            }
                        }

                        Item { Layout.preferredHeight: Theme.spacingLarge }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBottom

                    Controls.Button {
                        text: qsTr("Cancel")
                        visible: root.hasImportController && importController.isRunning
                        enabled: root.hasImportController && importController.isRunning
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: if (root.hasImportController) importController.cancelImport()
                    }

                    Controls.Button {
                        text: qsTr("Cancel all")
                        visible: root.hasImportController && importController.isRunning && importController.queuedCount > 0
                        enabled: visible
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: if (root.hasImportController) importController.cancelAllImports()
                    }

                    Item { Layout.fillWidth: true }

                    Controls.Button {
                        text: qsTr("Reset")
                        visible: root.hasImportController && !importController.isRunning
                        enabled: root.hasImportController && !importController.isRunning
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: { if (root.hasImportController) importController.resetStatus() }
                    }

                    Controls.Button {
                        text: qsTr("Start")
                        visible: root.hasImportController && !importController.isRunning
                        enabled: root.hasImportController && !importController.isRunning && ((importController.selectedFile && importController.selectedFile.length > 0) || importController.queuedCount > 0)
                        onClicked: { if (root.hasImportController) importController.startStatementImport() }
                    }

                    BusyIndicator {
                        running: root.hasImportController && importController.isRunning
                        visible: running
                        width: Theme.busyIndicatorSize
                        height: Theme.busyIndicatorSize
                    }
                }
            }
        }
    }

    Component {
        id: statementDraftPageComponent

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall

            StatementDraftView {
                id: stmtView
                Layout.fillWidth: true
                Layout.fillHeight: true
                draft: (root.hasImportController ? importController.draft : null)
            }

            RowLayout {
                Layout.fillWidth: true

                Controls.Button {
                    text: qsTr("Previous")
                    enabled: stmtView.draft && stmtView.draft.currentIndex > 0
                    onClicked: {
                        if (stmtView.draft) stmtView.draft.prev()
                        stmtView.forceSync && stmtView.forceSync()
                    }
                }

                Controls.Button {
                    text: qsTr("Next")
                    enabled: stmtView.draft && (stmtView.draft.currentIndex < stmtView.draft.count - 1)
                    onClicked: {
                        if (stmtView.draft) stmtView.draft.next()
                        stmtView.forceSync && stmtView.forceSync()
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
