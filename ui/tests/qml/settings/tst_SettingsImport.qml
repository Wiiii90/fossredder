/**
 * @file ui/tests/qml/settings/tst_SettingsImport.qml
 * @brief Provides QML tests for SettingsImport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "SettingsImportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsController: QtObject {
        property string importDefaultPath: ""
        property string importPoppler: ""
        property string importOpenCv: ""
        property string importTesseract: ""
        property string importParser: ""
        property string importMatcher: ""
    }

    property var actions: QtObject {
        property int browseCalls: 0
        signal importFileSelected(string path)
        signal importFilesSelected(var paths)
        function browseImportPdf() { browseCalls += 1 }
    }

    property var appContext: QtObject {
        property var settingsController: testCase.settingsController
        property var actions: testCase.actions
    }

    property var theme: QtObject {
        property int viewFormSpacing: 8
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsImportComponent
        SettingsImport {
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
        return createTemporaryObject(settingsImportComponent, testCase)
    }

    function init() {
        settingsController.importDefaultPath = ""
        actions.browseCalls = 0
    }

    function test_SET_I_001_defaultPathFieldUpdatesSettings() {
        var view = createView()
        var pathField = findRequired(view, "settingsImportDefaultPathField")

        pathField.text = "test:///import/default.pdf"

        compare(settingsController.importDefaultPath, "test:///import/default.pdf")
    }

    function test_SET_I_002_browseButtonCallsAction() {
        var view = createView()
        var browseButton = findRequired(view, "settingsImportBrowseButton")

        browseButton.clicked()

        compare(actions.browseCalls, 1)
    }

    function test_SET_I_003_selectedFileSignalUpdatesDefaultPath() {
        var view = createView()

        actions.importFileSelected("test:///import/from-action.pdf")

        compare(settingsController.importDefaultPath, "test:///import/from-action.pdf")
    }

}
