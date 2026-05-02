/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Export/ExportView.qml
 * @brief Provides the ExportView component.
 */

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
    readonly property var exportCtrl: root.appContext ? root.appContext.exportController : null
    readonly property var actions: root.appContext ? root.appContext.actions : null
    readonly property var fileSystemController: root.appContext ? root.appContext.fileSystemController : null
    readonly property var settingsController: root.appContext ? root.appContext.settingsController : null
    readonly property bool hasExportCtrl: root.exportCtrl !== null
    readonly property string readyText: qsTr("Ready")
    readonly property int csvContractValue: 0
    readonly property int xlsxContractValue: 1
    readonly property string defaultLocale: Qt.locale().name.replace("_", "-")
    readonly property var formatOptions: [
        {
            contract: csvContractValue,
            extension: Constants.FileFormats.exportFormats.csv.extension,
            label: Constants.FileFormats.exportFormats.csv.label
        },
        {
            contract: xlsxContractValue,
            extension: Constants.FileFormats.exportFormats.xlsx.extension,
            label: Constants.FileFormats.exportFormats.xlsx.label
        }
    ]
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    function buildPayload() {
        const targetDirectory = formPanel ? formPanel.targetDirectory : ""
        const packageFormatIndex = formPanel ? formPanel.packageFormatIndex : 0
        const items = objectsPanel ? objectsPanel.exportItems() : []
        const payload = {
            targetDirectory: targetDirectory,
            packageFormatIndex: packageFormatIndex,
            items: items
        }
        return JSON.stringify(payload)
    }

    function defaultTargetDirectory() {
        if (root.settingsController && root.settingsController.exportDefaultDirectory && root.settingsController.exportDefaultDirectory.length > 0)
            return String(root.settingsController.exportDefaultDirectory)
        if (root.fileSystemController && root.fileSystemController.appDir) {
            const appPath = root.fileSystemController.appDir()
            if (appPath && String(appPath).length > 0) return String(appPath)
        }
        return Qt.resolvedUrl(".").toString().replace("file:///", "")
    }

    function clearForm() {
        if (formPanel) {
            formPanel.targetDirectory = root.defaultTargetDirectory()
            formPanel.packageFormatIndex = root.settingsController ? root.settingsController.exportArchiveFormat : 0
        }
        if (objectsPanel) objectsPanel.clearAll()
        if (root.hasExportCtrl) root.exportCtrl.clearActiveRun()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacing

        Flickable {
            id: exportScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: Math.max(exportContent.implicitHeight, exportScroll.height)

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: exportContent
                width: exportScroll.width
                height: Math.max(implicitHeight, exportScroll.height)
                spacing: root.theme.spacing

                ExportForm {
                    id: formPanel
                    Layout.fillWidth: true
                    theme: root.theme
                    Component.onCompleted: {
                        if (!targetDirectory || targetDirectory.length === 0)
                            targetDirectory = root.defaultTargetDirectory()
                        packageFormatIndex = root.settingsController ? root.settingsController.exportArchiveFormat : 0
                    }
                    onBrowseRequested: {
                        if (root.actions) root.actions.browseExportDirectory()
                    }
                }

                ExportPanel {
                    id: objectsPanel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 320
                    appContext: root.appContext
                    theme: root.theme
                }
            }
        }

        ExportProgressBar {
            Layout.fillWidth: true
            theme: root.theme
            exportCtrl: root.exportCtrl
            hasExportCtrl: root.hasExportCtrl
            readyText: root.readyText
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.SecondaryButton {
                text: qsTr("Clear")
                visible: root.hasExportCtrl && root.exportCtrl.currentMode === 0
                onClicked: root.clearForm()
            }

            Controls.SecondaryButton {
                text: qsTr("Cancel")
                visible: root.hasExportCtrl && root.exportCtrl.currentMode === 1
                onClicked: if (root.hasExportCtrl) root.exportCtrl.cancelExport()
            }

            Item { Layout.fillWidth: true }

            Controls.SecondaryButton {
                text: root.hasExportCtrl && root.exportCtrl.isPaused ? qsTr("Resume") : qsTr("Pause")
                visible: root.hasExportCtrl && root.exportCtrl.currentMode === 1
                onClicked: if (root.hasExportCtrl) root.exportCtrl.togglePause()
            }

            Controls.SuccessButton {
                text: qsTr("Start")
                visible: root.hasExportCtrl && root.exportCtrl.currentMode === 0
                enabled: {
                    const dir = formPanel ? formPanel.targetDirectory : ""
                    const items = objectsPanel ? objectsPanel.exportItems() : []
                    return dir.length > 0 && items.length > 0
                }
                onClicked: {
                    if (!root.hasExportCtrl) return
                    const payload = root.buildPayload()
                    const path = (formPanel ? formPanel.targetDirectory : "") + "/export"
                    const selectedOption = root.formatOptions[0]
                    const items = objectsPanel ? objectsPanel.exportItems() : []
                    const includeFormulas = root.settingsController ? root.settingsController.exportIncludeFormulas : true
                    root.exportCtrl.exportDataWithPayload(selectedOption.contract, path, includeFormulas, root.defaultLocale, payload, Math.max(1, items.length))
                }
            }
        }

        Connections {
            target: root.actions
            function onExportDirectorySelected(path) {
                if (!path) return
                if (formPanel) formPanel.targetDirectory = path
            }
        }

        Connections {
            target: root.settingsController
            function onExportDefaultDirectoryChanged() {
                if (formPanel && (!formPanel.targetDirectory || formPanel.targetDirectory.length === 0))
                    formPanel.targetDirectory = root.defaultTargetDirectory()
            }
            function onExportArchiveFormatChanged() {
                if (formPanel)
                    formPanel.packageFormatIndex = root.settingsController.exportArchiveFormat
            }
        }

    }
}
