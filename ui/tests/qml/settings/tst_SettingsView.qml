/**
 * @file ui/tests/qml/settings/tst_SettingsView.qml
 * @brief Provides QML tests for SettingsView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "SettingsViewTests"
    when: windowShown
    width: 960
    height: 640

    property var navigation: QtObject {
        property int settingsCategoryValue: 0
        signal settingsCategoryChanged()
        function setSettingsCategoryValue(value) {
            settingsCategoryValue = value
            settingsCategoryChanged()
        }
    }

    property var settingsController: QtObject {
        property string language: "en"
        property string importDefaultPath: ""
        property string importPoppler: ""
        property string importOpenCv: ""
        property string importTesseract: ""
        property string importParser: ""
        property string importMatcher: ""
        property string exportDefaultDirectory: ""
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
        property bool toolbarShowBooking: true
        property bool toolbarShowImport: true
        property bool toolbarShowActors: true
        property bool toolbarShowExport: true
        property bool toolbarShowProperties: true
        property bool toolbarShowAnalysis: true
        property bool toolbarShowContracts: true
        property bool toolbarShowAnnual: true
        property bool toolbarShowSettings: true
        property int saveCalls: 0
        property int resetCalls: 0
        property int loadCalls: 0
        function save() { saveCalls += 1 }
        function resetToDefaults() { resetCalls += 1 }
        function load() { loadCalls += 1 }
    }

    property var languageController: QtObject {
        property var availableLanguages: [
            { code: "en", label: "English", available: true },
            { code: "de", label: "Deutsch", available: true }
        ]
        property string currentLanguage: "en"
        property int applyCalls: 0
        property string lastLanguage: ""
        function applyLanguage(language) {
            applyCalls += 1
            lastLanguage = language
            currentLanguage = language
        }
    }

    property var actions: QtObject {
        signal importFileSelected(string path)
        signal importFilesSelected(var paths)
        signal exportDirectorySelected(string path)
        function browseImportPdf() {}
        function browseExportDirectory() {}
    }

    property var appContext: QtObject {
        property var navigation: testCase.navigation
        property var settingsController: testCase.settingsController
        property var languageController: testCase.languageController
        property var actions: testCase.actions
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int formLabelWidth: 120
        property int viewActionButtonWidth: 120
        property int controlHeight: 32
        property int borderWidthThin: 1
        property int radius: 3
        property color subtlePrimaryFill: "#eef3ff"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsViewComponent
        SettingsView {
            width: 960
            height: 640
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
        return createTemporaryObject(settingsViewComponent, testCase)
    }

    function init() {
        navigation.settingsCategoryValue = 0
        settingsController.saveCalls = 0
        settingsController.resetCalls = 0
        settingsController.loadCalls = 0
        settingsController.language = "en"
        languageController.applyCalls = 0
        languageController.lastLanguage = ""
    }

    function test_SET_V_003_updateButtonSavesAndAppliesLanguage() {
        var view = createView()
        var updateButton = findRequired(view, "settingsUpdateButton")

        settingsController.language = "de"
        updateButton.clicked()

        compare(settingsController.saveCalls, 1)
        compare(languageController.applyCalls, 1)
        compare(languageController.lastLanguage, "de")
    }

    function test_SET_V_004_defaultButtonResetsSettingsAndCategory() {
        navigation.settingsCategoryValue = 2
        var view = createView()
        var defaultButton = findRequired(view, "settingsDefaultButton")

        defaultButton.clicked()

        compare(settingsController.resetCalls, 1)
        compare(navigation.settingsCategoryValue, 0)
    }

    function test_SET_V_002_categoryNavigationButtonsAdvanceAndReturn() {
        var view = createView()
        var nextButton = findRequired(view, "settingsNextCategoryButton")
        var prevButton = findRequired(view, "settingsPrevCategoryButton")
        var loader = findRequired(view, "settingsLoader")

        nextButton.clicked()
        compare(navigation.settingsCategoryValue, 1)
        verify(loader.item !== null)

        prevButton.clicked()
        compare(navigation.settingsCategoryValue, 0)
        verify(loader.item !== null)
    }
}
