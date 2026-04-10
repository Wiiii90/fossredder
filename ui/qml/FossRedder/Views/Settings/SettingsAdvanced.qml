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
                    Label { text: qsTr("Diagnostics"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Logging is handled by the central error reporting pipeline and not configured from the UI yet.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Build mode hint"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: AppContext.isDebugBuild
                              ? qsTr("Debug-specific UI diagnostics are available in debug builds.")
                              : qsTr("This build runs with the production-oriented UI configuration.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
