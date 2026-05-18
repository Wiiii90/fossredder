/**
 * @file ui/tests/qml/settings/tst_SettingsGeneral.qml
 * @brief Provides QML tests for SettingsGeneral behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "SettingsGeneralTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property string language: "en"
    }

    property var languageService: QtObject {
        property var availableLanguages: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        property string currentLanguage: "en"
    }

    property var appContext: QtObject {
        property var settingsViewModel: testCase.settingsViewModel
        property var languageService: testCase.languageService
    }

    property var theme: QtObject {
        property int viewFormSpacing: 8
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsGeneralComponent
        SettingsGeneral {
            width: 900
            height: 560
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(settingsGeneralComponent, testCase)
    }

    function init() {
        languageService.availableLanguages = [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        settingsViewModel.language = "en"
        languageService.currentLanguage = "en"
    }

    function test_SET_G_003_languageDropdownReflectsSettingsLanguage() {
        var view = createView()
        var languageDropdown = findRequired(view, "settingsLanguageDropdown")

        settingsViewModel.language = "de"
        languageService.currentLanguage = "de"

        tryCompare(languageDropdown, "currentIndex", 1)
    }

    function test_SET_G_001_languageSelectionUpdatesSettingsLanguage() {
        var view = createView()
        var languageDropdown = findRequired(view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsViewModel.language, "de")
    }

    function test_SET_G_002_unavailableLanguageSelectionReverts() {
        languageService.availableLanguages = [
            { code: "en", label: "English", available: true },
            { code: "xx", label: "Unavailable", available: false }
        ]
        settingsViewModel.language = "en"

        var view = createView()
        var languageDropdown = findRequired(view, "settingsLanguageDropdown")

        languageDropdown.currentIndex = 1
        languageDropdown.activated(1)

        compare(settingsViewModel.language, "en")
        compare(languageDropdown.currentIndex, 0)
    }

}
