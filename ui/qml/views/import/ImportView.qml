import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import components.controls 1.0

Item {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    anchors.margins: 8

    property bool hasUiImport: typeof uiImport !== 'undefined'

    property bool showAdvanced: false
    property bool showPoppler: false
    property bool showTesseract: false
    property bool showParser: false

    property var pendingFiles: []

    function commitImportFiles(paths, updatePathField) {
        if (!paths || paths.length === 0) return

        var pdfs = []
        for (var i = 0; i < paths.length; ++i) {
            var p = String(paths[i])
            if (!p || p.length === 0) continue
            if (p.toLowerCase().endsWith(".pdf")) pdfs.push(p)
        }
        if (pdfs.length === 0) return

        if (updatePathField && manualPath) manualPath.text = pdfs[0]
        if (hasUiImport) uiImport.addFiles(pdfs)
        pendingFiles = []
    }

    function addImportFiles(paths) {
        commitImportFiles(paths, true)
    }

    StackLayout {
        id: contentStack
        anchors.fill: parent
        currentIndex: (hasUiImport && uiImport.draft) ? 1 : 0

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

                        AppPanel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("Source"); Layout.preferredWidth: 120 }
                                    AppComboBox {
                                        id: sourceKind
                                        model: [qsTr("PDF")]
                                        currentIndex: 0
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: qsTr("Strategy"); Layout.preferredWidth: 120 }
                                    AppComboBox {
                                        id: strategy
                                        model: [qsTr("Commerzbank26")]
                                        currentIndex: 0
                                    }
                                }

                                AppCheckBox {
                                    text: qsTr("Advanced settings")
                                    checked: root.showAdvanced
                                    onToggled: root.showAdvanced = checked
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    visible: root.showAdvanced
                                    spacing: Theme.spacingSmall

                                    AppCheckBox {
                                        text: qsTr("Poppler")
                                        checked: root.showPoppler
                                        onToggled: root.showPoppler = checked
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        visible: root.showPoppler
                                        Layout.leftMargin: Theme.spacing
                                        Label { text: qsTr("TODO: Poppler settings"); color: Theme.textMuted; wrapMode: Text.WordWrap }
                                    }

                                    AppCheckBox {
                                        text: qsTr("Tesseract")
                                        checked: root.showTesseract
                                        onToggled: root.showTesseract = checked
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        visible: root.showTesseract
                                        Layout.leftMargin: Theme.spacing
                                        Label { text: qsTr("TODO: Tesseract settings"); color: Theme.textMuted; wrapMode: Text.WordWrap }
                                    }

                                    AppCheckBox {
                                        text: qsTr("Parser")
                                        checked: root.showParser
                                        onToggled: root.showParser = checked
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        visible: root.showParser
                                        Layout.leftMargin: Theme.spacing
                                        Label { text: qsTr("TODO: Parser settings"); color: Theme.textMuted; wrapMode: Text.WordWrap }
                                    }
                                }
                        }

                        AppPanel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                                RowLayout {
                                    Layout.fillWidth: true

                                    AppTextField {
                                        id: manualPath
                                        Layout.fillWidth: true
                                        placeholderText: qsTr("Enter file path...")
                                        enabled: hasUiImport && !uiImport.isRunning

                                        Component.onCompleted: {
                                            var def = "P:/.data/fossredder/April 2025.pdf"
                                            text = def
                                            if (hasUiImport) uiImport.selectedFile = ""
                                            pendingFiles = []
                                        }

                                        onTextEdited: { pendingFiles = [] }
                                    }

                                    AppButton {
                                        text: qsTr("Add")
                                        enabled: hasUiImport && !uiImport.isRunning && manualPath.text && manualPath.text.trim().length > 0
                                        fillColor: Theme.surface
                                        textColor: Theme.textPrimary
                                        onClicked: {
                                            var files = []
                                            if (pendingFiles && pendingFiles.length > 0) files = pendingFiles
                                            else files = [manualPath.text]
                                            root.commitImportFiles(files, false)

                                            manualPath.text = ""
                                            pendingFiles = []
                                        }
                                    }

                                    AppButton {
                                        text: qsTr("Browse...")
                                        enabled: hasUiImport && !uiImport.isRunning
                                        fillColor: Theme.surface
                                        textColor: Theme.textPrimary
                                        onClicked: { if (uiActions) uiActions.browseImportPdf() }
                                    }
                                }

                                AppDropZone {
                                    Layout.fillWidth: true
                                    enabled: hasUiImport && !uiImport.isRunning
                                    title: qsTr("Drop PDFs here")
                                    subtitle: ""
                                    allowBrowse: false
                                    clickToBrowse: true
                                    queuedCount: hasUiImport ? uiImport.queuedCount : 0
                                    files: hasUiImport ? (uiImport.selectedFile && uiImport.selectedFile.length > 0 ? [uiImport.selectedFile].concat(uiImport.queuedFiles) : uiImport.queuedFiles) : []
                                    onBrowseRequested: { if (uiActions) uiActions.browseImportPdf() }
                                }

                                Connections {
                                    target: uiActions
                                    function onImportFileSelected(path) {
                                        if (!path) return
                                        manualPath.text = path
                                        pendingFiles = [path]
                                    }
                                    function onImportFilesSelected(paths) {
                                        if (!paths || paths.length === 0) return
                                        manualPath.text = paths[0]
                                        pendingFiles = paths
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

                        }

                        AppPanel {
                            Layout.fillWidth: true
                            contentSpacing: Theme.spacingSmall

                                AppProgressBar {
                                    Layout.fillWidth: true
                                    visible: hasUiImport && (uiImport.isRunning || uiImport.progress > 0)
                                    value: hasUiImport ? uiImport.progress : 0
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: hasUiImport ? (uiImport.error && uiImport.error.length > 0 ? uiImport.error : uiImport.phase) : ""
                                    color: hasUiImport && uiImport.error && uiImport.error.length > 0 ? Theme.danger : Theme.textPrimary
                                    wrapMode: Text.WordWrap
                                }

                                Label {
                                    Layout.fillWidth: true
                                    visible: hasUiImport ? (uiImport.isRunning && uiImport.pageCount > 0) : false
                                    text: hasUiImport ? qsTr("Page %1/%2").arg(uiImport.currentPage).arg(uiImport.pageCount) : ""
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

                    AppButton {
                        text: qsTr("Cancel")
                        visible: hasUiImport && uiImport.isRunning
                        enabled: hasUiImport && uiImport.isRunning
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: if (hasUiImport) uiImport.cancelImport()
                    }

                    AppButton {
                        text: qsTr("Cancel all")
                        visible: hasUiImport && uiImport.isRunning && uiImport.queuedCount > 0
                        enabled: visible
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: if (hasUiImport) uiImport.cancelAllImports()
                    }

                    Item { Layout.fillWidth: true }

                    AppButton {
                        text: qsTr("Reset")
                        visible: hasUiImport && !uiImport.isRunning
                        enabled: hasUiImport && !uiImport.isRunning
                        fillColor: Theme.surface
                        textColor: Theme.textPrimary
                        onClicked: { if (hasUiImport) uiImport.resetStatus() }
                    }

                    AppButton {
                        text: qsTr("Start")
                        visible: hasUiImport && !uiImport.isRunning
                        enabled: hasUiImport && !uiImport.isRunning && ((uiImport.selectedFile && uiImport.selectedFile.length > 0) || uiImport.queuedCount > 0)
                        onClicked: { if (hasUiImport) uiImport.startStatementImport() }
                    }

                    BusyIndicator {
                        running: hasUiImport && uiImport.isRunning
                        visible: running
                        width: 24
                        height: 24
                    }
                }
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: stmtLayout.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            ColumnLayout {
                id: stmtLayout
                width: parent.width

                StatementDraftView {
                    id: stmtView
                    Layout.fillWidth: true
                    draft: (hasUiImport ? uiImport.draft : null)
                }
            }
        }
    }
}
