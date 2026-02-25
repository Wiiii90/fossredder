import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Loader {
            id: settingsLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "qrc:/qml/views/settings/SettingsGeneral.qml"
        }

        Connections {
            target: uiNav
            function onSettingsCategoryChanged() {
                if (!settingsLoader || !uiNav) return
                var c = uiNav.settingsCategory
                switch (c) {
                case 0: settingsLoader.source = "qrc:/qml/views/settings/SettingsGeneral.qml"; break
                case 1: settingsLoader.source = "qrc:/qml/views/settings/SettingsAppearance.qml"; break
                case 2: settingsLoader.source = "qrc:/qml/views/settings/SettingsImport.qml"; break
                case 3: settingsLoader.source = "qrc:/qml/views/settings/SettingsExport.qml"; break
                case 4: settingsLoader.source = "qrc:/qml/views/settings/SettingsAdvanced.qml"; break
                default: settingsLoader.source = "qrc:/qml/views/settings/SettingsGeneral.qml"; break
                }
            }
        }

        Component.onCompleted: {
            if (uiNav) {
                uiNav.settingsCategory = uiNav.settingsCategory
            }
        }
    }
}
