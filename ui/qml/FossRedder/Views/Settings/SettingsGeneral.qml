/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Settings/SettingsGeneral.qml
 * @brief Provides the SettingsGeneral component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var settingsController: root.appContext ? root.appContext.settingsController : null
    readonly property var languageController: root.appContext ? root.appContext.languageController : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    contentWidth: width
    clip: true

    function languageIndexFor(code) {
        if (!root.languageController || !root.languageController.availableLanguages) return -1
        for (let i = 0; i < root.languageController.availableLanguages.length; ++i) {
            const option = root.languageController.availableLanguages[i]
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
                        objectName: "settingsLanguageComboBox"
                        model: root.languageController ? root.languageController.availableLanguages : []
                        textRole: "label"
                        currentIndex: root.languageIndexFor(root.settingsController ? root.settingsController.language : (root.languageController ? root.languageController.currentLanguage : ""))
                        onActivated: function(index) {
                            if (!root.languageController || index < 0 || index >= model.length) return
                            const option = model[index]
                            if (!option || option.available === false) {
                                currentIndex = root.languageIndexFor(root.settingsController ? root.settingsController.language : root.languageController.currentLanguage)
                                return
                            }
                            if (root.settingsController)
                                root.settingsController.language = option.code
                        }

                        Connections {
                            target: root.settingsController
                            function onLanguageChanged() {
                                language.currentIndex = root.languageIndexFor(root.settingsController.language)
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
