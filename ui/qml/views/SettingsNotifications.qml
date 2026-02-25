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

        Label { text: qsTr("Notifications"); font.pointSize: 18 }

        GroupBox {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Show error notifications"); Layout.fillWidth: true }
                    AppCheckBox { checked: true }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Check for updates automatically"); Layout.fillWidth: true }
                    AppCheckBox { checked: true }
                }
            }
        }

    }
}
