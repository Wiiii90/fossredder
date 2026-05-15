/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsImport.qml
 * @brief Configures import defaults and pipeline-related import settings.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var settingsViewModel: root.appContext ? root.appContext.settingsViewModel : null
    readonly property var actions: root.appContext ? root.appContext.actions : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

    ColumnLayout {
        id: column
        anchors.fill: parent
        width: parent.width
        spacing: root.theme.viewFormSpacing

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Default file path"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        id: defaultImportPathField
                        objectName: "settingsImportDefaultPathField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Select default import path...")
                        text: root.settingsViewModel ? root.settingsViewModel.importDefaultPath : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importDefaultPath !== text) root.settingsViewModel.importDefaultPath = text
                    }
                    Controls.SecondaryButton {
                        objectName: "settingsImportBrowseButton"
                        text: qsTr("Browse...")
                        Layout.preferredHeight: defaultImportPathField.implicitHeight
                        onClicked: if (root.actions) root.actions.browseImportPdf()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Import defaults"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("This path is used to prefill the Import view so repeated imports start from a consistent default.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Text {
                    text: qsTr("Pipeline configuration")
                    color: root.theme.textPrimary
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Poppler"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for Poppler defaults")
                        text: root.settingsViewModel ? root.settingsViewModel.importPoppler : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importPoppler !== text) root.settingsViewModel.importPoppler = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("OpenCV"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for OpenCV defaults")
                        text: root.settingsViewModel ? root.settingsViewModel.importOpenCv : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importOpenCv !== text) root.settingsViewModel.importOpenCv = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Tesseract"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for Tesseract defaults")
                        text: root.settingsViewModel ? root.settingsViewModel.importTesseract : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importTesseract !== text) root.settingsViewModel.importTesseract = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Parser"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for parser defaults")
                        text: root.settingsViewModel ? root.settingsViewModel.importParser : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importParser !== text) root.settingsViewModel.importParser = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Matcher"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Placeholder for matcher defaults")
                        text: root.settingsViewModel ? root.settingsViewModel.importMatcher : ""
                        onTextChanged: if (root.settingsViewModel && root.settingsViewModel.importMatcher !== text) root.settingsViewModel.importMatcher = text
                    }
                }
            }
        }

        Connections {
            target: root.actions
            function onImportFileSelected(path) {
                if (!path || !root.settingsViewModel) return
                root.settingsViewModel.importDefaultPath = path
            }
            function onImportFilesSelected(paths) {
                if (!paths || paths.length === 0 || !root.settingsViewModel) return
                root.settingsViewModel.importDefaultPath = paths[0]
            }
        }

    }
}
