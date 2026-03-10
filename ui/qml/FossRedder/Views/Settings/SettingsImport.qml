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
                    Label { text: qsTr("Supported source format"); Layout.fillWidth: true }
                    Label { text: FileFormats.importSources.pdf.label; color: Theme.textPrimary }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Pipeline configuration"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("PDF import is currently configured automatically by the application pipeline.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
