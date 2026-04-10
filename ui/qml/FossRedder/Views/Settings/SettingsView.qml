import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.settings.spacing

        Loader {
            id: settingsLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: generalComp
        }

        Component { id: generalComp; Views.SettingsGeneral { } }
        Component { id: appearanceComp; Views.SettingsAppearance { } }
        Component { id: importComp; Views.SettingsImport { } }
        Component { id: exportComp; Views.SettingsExport { } }
        Component { id: advancedComp; Views.SettingsAdvanced { } }

        Connections {
            target: navigation
            function onSettingsCategoryChanged() {
                if (!settingsLoader || !navigation) return
                var c = navigation.settingsCategory
                switch (c) {
                case 0: settingsLoader.sourceComponent = generalComp; break
                case 1: settingsLoader.sourceComponent = appearanceComp; break
                case 2: settingsLoader.sourceComponent = importComp; break
                case 3: settingsLoader.sourceComponent = exportComp; break
                case 4: settingsLoader.sourceComponent = advancedComp; break
                default: settingsLoader.sourceComponent = generalComp; break
                }
            }
        }

        Component.onCompleted: {
            if (navigation) {
                navigation.settingsCategory = navigation.settingsCategory
            }
        }
    }
}

