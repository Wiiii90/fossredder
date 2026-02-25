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

        Label { text: qsTr("Settings"); font.pointSize: 18; color: Theme.textPrimary }

        // loader for the selected settings category view
        Loader {
            id: settingsLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "qrc:/qml/views/SettingsGeneral.qml"
        }

        // react to uiNav.settingsCategory changes
        Connections {
            target: uiNav
            function onSettingsCategoryChanged() {
                if (!settingsLoader) return
                try {
                    var c = uiNav ? uiNav.settingsCategory : 0
                    switch (c) {
                    case 0: settingsLoader.source = "qrc:/qml/views/SettingsGeneral.qml"; break
                    case 1: settingsLoader.source = "qrc:/qml/views/SettingsAppearance.qml"; break
                    case 2: settingsLoader.source = "qrc:/qml/views/SettingsImport.qml"; break
                    case 3: settingsLoader.source = "qrc:/qml/views/SettingsExport.qml"; break
                    case 4: settingsLoader.source = "qrc:/qml/views/SettingsStorage.qml"; break
                    case 5: settingsLoader.source = "qrc:/qml/views/SettingsNotifications.qml"; break
                    case 6: settingsLoader.source = "qrc:/qml/views/SettingsAdvanced.qml"; break
                    case 7: settingsLoader.source = "qrc:/qml/views/SettingsUpdates.qml"; break
                    default: settingsLoader.source = "qrc:/qml/views/SettingsGeneral.qml"; break
                    }
                } catch(e) { }
            }
        }

        Component.onCompleted: {
            // ensure initial selection reflects uiNav state
            try { if (uiNav) { uiNav.settingsCategory = uiNav.settingsCategory } } catch(e) {}
        }
    }
}
