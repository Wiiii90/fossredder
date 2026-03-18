import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
    clip: true

    function languageIndexFor(code) {
        if (!languageController || !languageController.availableLanguages) return -1
        for (var i = 0; i < languageController.availableLanguages.length; ++i) {
            var option = languageController.availableLanguages[i]
            if (option && option.code === code) return i
        }
        return -1
    }

    ColumnLayout {
        id: column
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        spacing: Theme.settings.spacing
        anchors.margins: Theme.settings.margin

        GroupBox {
            Layout.preferredWidth: Theme.settings.panelPreferredWidth
            Layout.alignment: Qt.AlignHCenter
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.settings.spacing

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Language"); Layout.fillWidth: true }
                    Controls.ComboBox {
                        id: language
                        objectName: "settingsLanguageComboBox"
                        model: languageController ? languageController.availableLanguages : []
                        textRole: "label"
                        currentIndex: root.languageIndexFor(languageController ? languageController.currentLanguage : "")
                        onActivated: function(index) {
                            if (!languageController || index < 0 || index >= model.length) return
                            var option = model[index]
                            if (!option || option.available === false) {
                                currentIndex = root.languageIndexFor(languageController.currentLanguage)
                                return
                            }
                            languageController.currentLanguage = option.code
                        }

                        Connections {
                            target: languageController
                            function onCurrentLanguageChanged() {
                                language.currentIndex = root.languageIndexFor(languageController.currentLanguage)
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Application directory"); Layout.fillWidth: true }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Controls.TextField {
                        id: appDirectoryField
                        objectName: "settingsAppDirectoryField"
                        Layout.fillWidth: true
                        readOnly: true
                        text: (typeof fileSystemController !== 'undefined' && fileSystemController)
                              ? fileSystemController.appDir()
                              : ""
                        placeholderText: qsTr("Application directory is not available")
                    }
                }
            }
        }
    }
}
