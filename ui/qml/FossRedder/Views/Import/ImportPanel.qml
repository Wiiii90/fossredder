/**
 * @file ui/qml/FossRedder/Views/Import/ImportPanel.qml
 * @brief Provides the ImportPanel component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var importWorkflow
    required property var actions
    required property var status

    readonly property bool hasImportWorkflow: root.importWorkflow !== null
    readonly property string importCanceledStatusText: qsTr("Import canceled")
    readonly property string importFinishedStatusText: qsTr("Import finished")
    readonly property string importFailedStatusText: qsTr("Import failed")

    property var pendingFiles: []

    function manualPathText() {
        return manualPath ? manualPath.text : ""
    }

    function commitImportFiles(paths) {
        if (!paths || paths.length === 0) return

        const pdfs = []
        for (let i = 0; i < paths.length; ++i) {
            const p = String(paths[i])
            if (!p || p.length === 0) continue
            if (Constants.FileFormats.isSupportedImportPath(p)) pdfs.push(p)
        }
        if (pdfs.length === 0) return

        if (root.hasImportWorkflow) root.importWorkflow.addFiles(pdfs)
        root.pendingFiles = []
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: 260
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true

        readonly property real actionButtonHeight: manualPath.implicitHeight

        Controls.TextField {
            id: manualPath
            objectName: "importManualPathField"
            Layout.fillWidth: true
            placeholderText: qsTr("Enter file path...")
            enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning
            onTextEdited: { root.pendingFiles = [] }
        }

        Controls.SecondaryButton {
            objectName: "importAddFileButton"
            text: qsTr("Add")
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning && root.manualPathText() && root.manualPathText().trim().length > 0
            onClicked: {
                let files = []
                if (root.pendingFiles && root.pendingFiles.length > 0) files = root.pendingFiles
                else files = [root.manualPathText()]
                root.commitImportFiles(files)
                manualPath.text = ""
                root.pendingFiles = []
            }
        }

        Controls.SecondaryButton {
            objectName: "importBrowseFileButton"
            text: qsTr("Browse...")
            Layout.preferredHeight: parent.actionButtonHeight
            enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning
            onClicked: { if (root.actions) root.actions.browseImportPdf() }
        }
    }

    Controls.DropZone {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 160
        enabled: root.hasImportWorkflow && !root.importWorkflow.isRunning
        title: qsTr("Drop PDFs here")
        subtitle: ""
        allowBrowse: false
        clickToBrowse: true
        queuedCount: root.hasImportWorkflow ? root.importWorkflow.queuedCount : 0
        files: root.hasImportWorkflow ? (root.importWorkflow.selectedFile && root.importWorkflow.selectedFile.length > 0 ? [root.importWorkflow.selectedFile].concat(root.importWorkflow.queuedFiles) : root.importWorkflow.queuedFiles) : []
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
            root.commitImportFiles([path])
        }
        function onImportFilesDropped(paths) {
            if (!paths || paths.length === 0) return
            root.commitImportFiles(paths)
        }
    }

    Connections {
        target: root.hasImportWorkflow ? root.importWorkflow : null
        function onStateChanged() {
            if (!manualPath.activeFocus && (!root.pendingFiles || root.pendingFiles.length === 0))
                manualPath.text = root.importWorkflow.selectedFile
        }
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
