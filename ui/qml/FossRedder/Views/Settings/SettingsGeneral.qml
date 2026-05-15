/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsGeneral.qml
 * @brief Manages general settings options such as application language.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var settingsViewModel: root.appContext ? root.appContext.settingsViewModel : null
    readonly property var languageService: root.appContext ? root.appContext.languageService : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

    function languageIndexFor(code) {
        if (!root.languageService || !root.languageService.availableLanguages) return -1
        for (let i = 0; i < root.languageService.availableLanguages.length; ++i) {
            const option = root.languageService.availableLanguages[i]
            if (option && option.code === code) return i
        }
        return -1
    }

    ColumnLayout {
        id: column
        anchors.fill: parent
        width: parent.width
        spacing: root.theme.viewFormSpacing

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Language"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.DropdownMenu {
                        id: language
        objectName: "settingsLanguageDropdown"
                        model: root.languageService ? root.languageService.availableLanguages : []
                        textRole: "label"
                        currentIndex: root.languageIndexFor(root.settingsViewModel ? root.settingsViewModel.language : (root.languageService ? root.languageService.currentLanguage : ""))
                        onActivated: function(index) {
                            if (!root.languageService || index < 0 || index >= model.length) return
                            const option = model[index]
                            if (!option || option.available === false) {
                                currentIndex = root.languageIndexFor(root.settingsViewModel ? root.settingsViewModel.language : root.languageService.currentLanguage)
                                return
                            }
                            if (root.settingsViewModel)
                                root.settingsViewModel.language = option.code
                        }

                        Connections {
                            target: root.settingsViewModel
                            function onLanguageChanged() {
                                language.currentIndex = root.languageIndexFor(root.settingsViewModel.language)
                            }
                        }
                    }
                }

            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Only English and German are supported.")
                    color: root.theme.textMuted
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Note"); color: root.theme.textPrimary; Layout.preferredWidth: root.theme.formLabelWidth }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Language changes are applied when you update the global settings.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
