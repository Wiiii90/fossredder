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
    readonly property var languageService: root.appContext ? root.appContext.languageService : null
    readonly property var settingsViewModel: root.appContext ? root.appContext.settingsViewModel : null
    readonly property Component generalPageComponent: generalComp
    readonly property Component importPageComponent: importComp
    readonly property Component exportPageComponent: exportComp
    readonly property Component miscellaneousPageComponent: miscellaneousComp

    readonly property int firstCategory: 0
    readonly property int lastCategory: 3
    readonly property int currentCategory: root.navigation ? root.navigation.settingsCategoryValue : root.firstCategory

    function navigateCategory(delta) {
        if (!root.navigation)
            return
        const nextCategory = Math.max(root.firstCategory, Math.min(root.lastCategory, root.currentCategory + delta))
        if (nextCategory === root.currentCategory)
            return
        root.navigation.setSettingsCategoryValue(nextCategory)
    }

    function saveSettings() {
        if (root.settingsViewModel)
            root.settingsViewModel.save()
        if (root.languageService && root.settingsViewModel)
            root.languageService.applyLanguage(root.settingsViewModel.language)
    }

    function resetSettings() {
        if (root.navigation)
            root.navigation.setSettingsCategoryValue(0)
        if (root.settingsViewModel)
            root.settingsViewModel.resetToDefaults()
    }

    function componentForCategory(category) {
        switch (category) {
        case 1:
            return root.importPageComponent
        case 2:
            return root.exportPageComponent
        case 3:
            return root.miscellaneousPageComponent
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
        spacing: root.theme.viewFormSpacing

        Loader {
            id: settingsLoader
            objectName: "settingsLoader"
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: root.componentForCategory(root.navigation ? root.navigation.settingsCategoryValue : 0)
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "settingsPrevCategoryButton"
                enabled: root.currentCategory > root.firstCategory
                onClicked: root.navigateCategory(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "settingsDefaultButton"
                text: qsTr("Default")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.resetSettings()
            }

            Controls.SuccessButton {
                objectName: "settingsUpdateButton"
                text: qsTr("Update")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.saveSettings()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "settingsNextCategoryButton"
                enabled: root.currentCategory < root.lastCategory
                onClicked: root.navigateCategory(1)
            }
        }

        Component { id: generalComp; Views.SettingsGeneral { appContext: root.appContext; theme: root.theme } }
        Component { id: importComp; Views.SettingsImport { appContext: root.appContext; theme: root.theme } }
        Component { id: exportComp; Views.SettingsExport { appContext: root.appContext; theme: root.theme } }
        Component { id: miscellaneousComp; SettingsMiscellaneous { appContext: root.appContext; theme: root.theme } }

        Connections {
            target: root.navigation
            function onSettingsCategoryChanged() {
                if (!settingsLoader || !root.navigation) return
                settingsLoader.sourceComponent = root.componentForCategory(root.navigation.settingsCategoryValue)
            }
        }

        Component.onCompleted: {
            if (root.settingsViewModel)
                root.settingsViewModel.load()
            if (root.navigation) {
                root.navigation.setSettingsCategoryValue(root.navigation.settingsCategoryValue)
            }
            settingsLoader.sourceComponent = root.componentForCategory(root.navigation ? root.navigation.settingsCategoryValue : 0)
        }
    }
}

