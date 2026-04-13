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
                    Label { text: qsTr("Supported formats"); Layout.fillWidth: true }
                    Label { text: Constants.FileFormats.supportedExportLabels().join(", "); color: root.theme.textPrimary }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Formula support"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Formulas are available for %1 exports.").arg(Constants.FileFormats.exportFormats.xlsx.label)
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Defaults"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Export options are currently selected per export action in the Export view.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
