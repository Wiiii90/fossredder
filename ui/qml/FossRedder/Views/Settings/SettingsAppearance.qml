import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

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
                    Label { text: qsTr("Theme source"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("The current appearance is defined centrally in Theme.qml.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Font sizing"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Typography currently follows the shared theme and is not configured per user yet.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
