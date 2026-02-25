import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

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
        spacing: 8
        anchors.margins: 8

        Label { text: qsTr("Updates & Version"); font.pointSize: 18 }

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Auto-update channel"); Layout.fillWidth: true }
                    AppComboBox { model: [qsTr("Stable"), qsTr("Beta"), qsTr("Nightly")]; currentIndex: 0 }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Check for updates now"); Layout.fillWidth: true }
                    AppButton { text: qsTr("Check") }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Application version"); Layout.fillWidth: true }
                    Label { text: "1.0.0" }
                }

                Label { text: qsTr("Release notes") }
                AppTextField { text: qsTr("Initial release with basic settings UI."); readOnly: true; Layout.fillWidth: true }
            }
        }

    }
}
