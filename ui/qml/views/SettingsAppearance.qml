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

        Label { text: qsTr("Appearance"); font.pointSize: 18 }

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Theme"); Layout.fillWidth: true }
                    AppComboBox { model: [qsTr("System"), qsTr("Light"), qsTr("Dark")]; currentIndex: 0 }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Font size"); Layout.fillWidth: true }
                    AppComboBox { model: ["Small", "Normal", "Large"]; currentIndex: 1 }
                }
            }
        }

    }
}
