/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Settings/SettingsImport.qml
 * @brief Provides the SettingsImport component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants

Flickable {
    id: root
    required property var theme
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    clip: true

    ColumnLayout {
        id: column
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        spacing: root.theme.settings.spacing
        anchors.margins: root.theme.settings.margin

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.settings.spacing

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Supported source format"); Layout.fillWidth: true }
                    Label { text: Constants.FileFormats.importSources.pdf.label; color: root.theme.textPrimary }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Pipeline configuration"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("PDF import is currently configured automatically by the application pipeline.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Advanced settings (temporary)")

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.settings.spacing

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Import advanced settings are temporarily stored here while the Import view stays simplified.")
                    color: root.theme.textMuted
                    wrapMode: Text.WordWrap
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Poppler, Tesseract and Parser are currently managed automatically by the pipeline.")
                    color: root.theme.textMuted
                    wrapMode: Text.WordWrap
                }
            }
        }

    }
}
