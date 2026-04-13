import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Flickable {
    id: root
    required property var appContext
    required property var theme
    readonly property var languageController: root.appContext ? root.appContext.languageController : null
    readonly property var fileSystemController: root.appContext ? root.appContext.fileSystemController : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: column.implicitHeight
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
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        spacing: root.theme.settings.spacing
        anchors.margins: root.theme.settings.margin

        GroupBox {
            Layout.preferredWidth: root.theme.settings.panelPreferredWidth
            Layout.alignment: Qt.AlignHCenter
            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.settings.spacing

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Language"); Layout.fillWidth: true }
                    Controls.ComboBox {
                        id: language
                        objectName: "settingsLanguageComboBox"
                        model: root.languageController ? root.languageController.availableLanguages : []
                        textRole: "label"
                        currentIndex: root.languageIndexFor(root.languageController ? root.languageController.currentLanguage : "")
                        onActivated: function(index) {
                            if (!root.languageController || index < 0 || index >= model.length) return
                            const option = model[index]
                            if (!option || option.available === false) {
                                currentIndex = root.languageIndexFor(root.languageController.currentLanguage)
                                return
                            }
                            root.languageController.currentLanguage = option.code
                        }

                        Connections {
                            target: root.languageController
                            function onCurrentLanguageChanged() {
                                language.currentIndex = root.languageIndexFor(root.languageController.currentLanguage)
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
                        text: root.fileSystemController
                              ? root.fileSystemController.appDir()
                              : ""
                        placeholderText: qsTr("Application directory is not available")
                    }
                }
            }
        }
    }
}
