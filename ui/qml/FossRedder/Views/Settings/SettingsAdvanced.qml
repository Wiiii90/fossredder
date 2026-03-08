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

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Logging level"); Layout.fillWidth: true }
                    Controls.ComboBox { model: [qsTr("Error"), qsTr("Warning"), qsTr("Info"), qsTr("Debug")]; currentIndex: 2 }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Enable debug mode"); Layout.fillWidth: true }
                    Controls.CheckBox { checked: false }
                }
            }
        }

    }
}
