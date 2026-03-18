import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "SettingsGeneralTests"
    when: windowShown
    width: 960
    height: 640

    property var languageController: QtObject {
        property string currentLanguage: "en"
        property var availableLanguages: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
    }

    property var fileSystemController: QtObject {
        function appDir() {
            return "/workspace/app"
        }
    }

    Component {
        id: settingsComponent

        SettingsGeneral {
            width: 960
            height: 640
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(settingsComponent, testCase)
    }

    function init() {
        languageController.currentLanguage = "en"
        languageController.availableLanguages = [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
    }

    function test_appDirectoryFieldShowsFileSystemAppDir() {
        var view = createView()
        var appDirectoryField = findRequired(view, "settingsAppDirectoryField")

        compare(appDirectoryField.text, "/workspace/app")
    }

    function test_languageSelectionUpdatesControllerProperty() {
        var view = createView()
        var comboBox = findRequired(view, "settingsLanguageComboBox")

        comboBox.currentIndex = 1
        comboBox.activated(1)

        compare(languageController.currentLanguage, "de")
        compare(comboBox.currentIndex, 1)
    }

    function test_unavailableLanguageSelectionRestoresCurrentIndex() {
        languageController.availableLanguages = [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: false }
        ]

        var view = createView()
        var comboBox = findRequired(view, "settingsLanguageComboBox")

        comboBox.currentIndex = 1
        comboBox.activated(1)

        compare(languageController.currentLanguage, "en")
        compare(comboBox.currentIndex, 0)
    }
}
