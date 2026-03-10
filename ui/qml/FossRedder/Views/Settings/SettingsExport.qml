import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "../../Constants/FileFormats.js" as FileFormats

Flickable {
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    clip: true

    ColumnLayout {
        id: column
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        spacing: Theme.settings.spacing
        anchors.margins: Theme.settings.margin

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.settings.spacing

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Supported formats"); Layout.fillWidth: true }
                    Label { text: FileFormats.supportedExportLabels().join(", "); color: Theme.textPrimary }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Formula support"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Formulas are available for %1 exports.").arg(FileFormats.exportFormats.xlsx.label)
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Defaults"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Export options are currently selected per export action in the Export view.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
