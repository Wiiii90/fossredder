/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsExport.qml
 * @brief Edits export defaults such as output directory and archive format.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var settingsController: root.appContext ? root.appContext.settingsController : null
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
                    Text { text: qsTr("Default output folder"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField {
                        objectName: "settingsExportDefaultDirectoryField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Select default output folder...")
                        text: root.settingsController ? root.settingsController.exportDefaultDirectory : ""
                        onTextChanged: if (root.settingsController && root.settingsController.exportDefaultDirectory !== text) root.settingsController.exportDefaultDirectory = text
                    }
                    Controls.SecondaryButton {
                        objectName: "settingsExportBrowseButton"
                        text: qsTr("Browse...")
                        onClicked: if (root.actions) root.actions.browseExportDirectory()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Default archive"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.DropdownMenu {
                        objectName: "settingsExportArchiveFormatComboBox"
                        model: [qsTr("None"), qsTr("ZIP")]
                        currentIndex: root.settingsController ? root.settingsController.exportArchiveFormat : 0
                        onActivated: function(index) {
                            if (root.settingsController)
                                root.settingsController.exportArchiveFormat = index
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("XLSX formulas"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.CheckBox {
                        objectName: "settingsExportIncludeFormulasCheckBox"
                        checked: root.settingsController ? root.settingsController.exportIncludeFormulas : true
                        text: qsTr("Use Excel formulas for totals when possible")
                        onToggled: if (root.settingsController) root.settingsController.exportIncludeFormulas = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Implementation status"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("XLSX formula support is feasible and now mapped as a saved default. Totals can be written as formulas instead of static values in the spreadsheet.")
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
                    Layout.fillWidth: true
                    text: qsTr("Saved defaults are limited to the current export workflow: output folder, archive mode and XLSX formula handling.")
                    color: root.theme.textMuted
                    wrapMode: Text.WordWrap
                }
            }
        }

        Connections {
            target: root.actions
            function onExportDirectorySelected(path) {
                if (!path || !root.settingsController)
                    return
                root.settingsController.exportDefaultDirectory = path
            }
        }

    }
}
