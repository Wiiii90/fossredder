/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Settings/SettingsAppearance.qml
 * @brief Provides the SettingsAppearance component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Flickable {
    id: root
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
                    Label { text: qsTr("Theme source"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("The current appearance is defined centrally in Theme.qml.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Font sizing"); Layout.fillWidth: true }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Typography currently follows the shared theme and is not configured per user yet.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
