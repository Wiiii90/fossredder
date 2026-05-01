/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Settings/SettingsAdvanced.qml
 * @brief Provides the SettingsAdvanced component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Flickable {
    id: root
    required property var appContext
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
                    Label { text: qsTr("Diagnostics"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Logging is handled by the central error reporting pipeline and not configured from the UI yet.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Build mode hint"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: root.appContext && root.appContext.isDebugBuild
                              ? qsTr("Debug-specific UI diagnostics are available in debug builds.")
                              : qsTr("This build runs with the production-oriented UI configuration.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
