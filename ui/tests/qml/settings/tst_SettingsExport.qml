/**
 * @file ui/tests/qml/settings/tst_SettingsExport.qml
 * @brief Provides QML tests for SettingsExport behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "SettingsExportTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsViewModel: QtObject {
        property string exportDefaultDirectory: ""
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
    }

    property var actions: QtObject {
        signal exportDirectorySelected(string path)
        property int browseCalls: 0
        function browseExportDirectory() { browseCalls += 1 }
    }

    property var appContext: QtObject {
        property var settingsViewModel: testCase.settingsViewModel
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
        id: settingsExportComponent
        SettingsExport {
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
        return createTemporaryObject(settingsExportComponent, testCase)
    }

    function init() {
        settingsViewModel.exportDefaultDirectory = ""
        settingsViewModel.exportArchiveFormat = 0
        settingsViewModel.exportIncludeFormulas = true
        actions.browseCalls = 0
    }

    function test_SET_E_001_defaultDirectoryFieldUpdatesSettings() {
        var view = createView()
        var pathField = findRequired(view, "settingsExportDefaultDirectoryField")

        pathField.text = "test:///export/out"

        compare(settingsViewModel.exportDefaultDirectory, "test:///export/out")
    }

    function test_SET_E_002_archiveFormatSelectionUpdatesSettings() {
        var view = createView()
        var archiveCombo = findRequired(view, "settingsExportArchiveFormatComboBox")

        archiveCombo.currentIndex = 1
        archiveCombo.activated(1)

        compare(settingsViewModel.exportArchiveFormat, 1)
    }

    function test_SET_E_003_includeFormulasToggleUpdatesSettings() {
        var view = createView()
        var formulasCheck = findRequired(view, "settingsExportIncludeFormulasCheckBox")

        formulasCheck.checked = false
        formulasCheck.toggled()

        compare(settingsViewModel.exportIncludeFormulas, false)
    }

    function test_SET_E_004_browseButtonAndSelectedDirectorySignal() {
        var view = createView()
        var browseButton = findRequired(view, "settingsExportBrowseButton")

        browseButton.clicked()
        compare(actions.browseCalls, 1)

        actions.exportDirectorySelected("test:///export/from-action")
        compare(settingsViewModel.exportDefaultDirectory, "test:///export/from-action")
    }

}
