import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

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

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            Label { text: qsTr("General Settings"); font.pointSize: 18; Layout.alignment: Qt.AlignHCenter }

            Rectangle { color: "transparent"; Layout.preferredWidth: 760 }

            GroupBox {
                Layout.preferredWidth: 760
                Layout.alignment: Qt.AlignHCenter
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Start application on system startup"); Layout.fillWidth: true }
                        Controls.CheckBox { id: startOnStartup; checked: false }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Language"); Layout.fillWidth: true }
                        Controls.ComboBox { id: language; model: ["English", "German"]; currentIndex: 0 }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Default paths"); Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Controls.TextField { id: defaultDataPathField; Layout.fillWidth: true; placeholderText: qsTr("Select default data folder") }
                        Controls.Button { text: qsTr("Browse"); onClicked: defaultPicker.open() }
                        Controls.FilePicker { id: defaultPicker; onAccepted: function(path) { defaultDataPathField.text = path } }
                    }

                }
            }
        }

    }
}
