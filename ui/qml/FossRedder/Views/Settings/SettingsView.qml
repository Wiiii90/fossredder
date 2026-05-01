/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsView.qml
 * @brief Provides the SettingsView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var navigation: root.appContext ? root.appContext.navigation : null
    readonly property var languageController: root.appContext ? root.appContext.languageController : null
    readonly property Component generalPageComponent: generalComp
    readonly property Component appearancePageComponent: appearanceComp
    readonly property Component importPageComponent: importComp
    readonly property Component exportPageComponent: exportComp
    readonly property Component advancedPageComponent: advancedComp

    function saveSettings() {
        if (settingsLoader && settingsLoader.item)
            settingsLoader.focus = true
    }

    function resetSettings() {
        if (root.navigation)
            root.navigation.setSettingsCategoryValue(0)
        if (root.languageController)
            root.languageController.currentLanguage = "en"
    }

    function componentForCategory(category) {
        switch (category) {
        case 1:
            return root.appearancePageComponent
        case 2:
            return root.importPageComponent
        case 3:
            return root.exportPageComponent
        case 4:
            return root.advancedPageComponent
        default:
            return root.generalPageComponent
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.settings.spacing

        Loader {
            id: settingsLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: root.componentForCategory(root.navigation ? root.navigation.settingsCategoryValue : 0)
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrimaryButton {
                text: qsTr("Save")
                onClicked: root.saveSettings()
            }

            Controls.SecondaryButton {
                text: qsTr("Default")
                onClicked: root.resetSettings()
            }

            Item { Layout.fillWidth: true }
        }

        Component { id: generalComp; Views.SettingsGeneral { appContext: root.appContext; theme: root.theme } }
        Component { id: appearanceComp; Views.SettingsAppearance { theme: root.theme } }
        Component { id: importComp; Views.SettingsImport { theme: root.theme } }
        Component { id: exportComp; Views.SettingsExport { theme: root.theme } }
        Component { id: advancedComp; Views.SettingsAdvanced { appContext: root.appContext; theme: root.theme } }

        Connections {
            target: root.navigation
            function onSettingsCategoryChanged() {
                if (!settingsLoader || !root.navigation) return
                settingsLoader.sourceComponent = root.componentForCategory(root.navigation.settingsCategoryValue)
            }
        }

        Component.onCompleted: {
            if (root.navigation) {
                root.navigation.setSettingsCategoryValue(root.navigation.settingsCategoryValue)
            }
            settingsLoader.sourceComponent = root.componentForCategory(root.navigation ? root.navigation.settingsCategoryValue : 0)
        }
    }
}

