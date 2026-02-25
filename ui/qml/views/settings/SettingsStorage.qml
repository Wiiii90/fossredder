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

        Label { text: qsTr("Storage & Privacy"); font.pointSize: 18 }

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    AppTextField { id: dbPathField; Layout.fillWidth: true; placeholderText: qsTr("Select database file") }
                    AppButton { text: qsTr("Browse"); onClicked: dbPicker.open() }
                    AppFilePicker { id: dbPicker; onAccepted: function(path) { dbPathField.text = path } }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Backup interval (days)"); Layout.fillWidth: true }
                    AppTextField { text: "7" }
                }
            }
        }

    }
}
