/**
 * @file ui/tests/qml/export/tst_ExportView.qml
 * @brief Provides QML tests for ExportView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportViewTests"
    when: windowShown
    width: 960
    height: 640

    property var actions: QtObject {
        property int browseCalls: 0
        signal exportDirectorySelected(string path)
        function browseExportDirectory() { browseCalls += 1 }
    }

    property var fileSystemController: QtObject {
        function appDir() { return "test:///runtime/app" }
    }

    property var settingsController: QtObject {
        property string exportDefaultDirectory: "test:///export/default"
        property int exportArchiveFormat: 0
        property bool exportIncludeFormulas: true
    }

    property var session: QtObject {
        property var annualsData: [
            { id: "annual-1", name: "Annual 1", display: "Annual 1", assignedAnalysisIds: ["analysis-1"] }
        ]
        property var analysesData: [
            { id: "analysis-1", name: "Analysis 1", type: "tab", exportFormat: "CSV" }
        ]
        function annualRows() { return annualsData || [] }
        function analysisRows() { return analysesData || [] }
    }

    property var exportController: QtObject {
        property int currentMode: 0
        property bool isPaused: false
        property real progress: 0
        property string phase: ""
        property string error: ""

        property int clearCalls: 0
        property int cancelCalls: 0
        property int togglePauseCalls: 0
        property int exportCalls: 0
        property var lastExportArgs: ({})

        function clearActiveRun() { clearCalls += 1 }
        function cancelExport() { cancelCalls += 1 }
        function togglePause() { togglePauseCalls += 1; isPaused = !isPaused }
        function exportDataWithPayload(contract, path, includeFormulas, locale, payload, count) {
            exportCalls += 1
            lastExportArgs = {
                contract: contract,
                path: path,
                includeFormulas: includeFormulas,
                locale: locale,
                payload: payload,
                count: count
            }
        }
    }

    property var appContext: QtObject {
        property var exportController: testCase.exportController
        property var actions: testCase.actions
        property var fileSystemController: testCase.fileSystemController
        property var settingsController: testCase.settingsController
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacing: 8
        property int spacingSmall: 6
        property int formLabelWidth: 120
        property int controlHeight: 32
        property int formFieldWidth: 220
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property int viewInlineIconSize: 28
        property int viewSectionIconSize: 42
        property int borderWidthThin: 1
        property int radius: 3
        property int margins: 8
        property color subtlePrimaryFill: "#eef3ff"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color danger: "#aa0000"
        property color warning: "#aa8800"
        property color success: "#008800"
        property string fontFamily: "Segoe UI"
        property int fontSize: 10
    }

    Component {
        id: exportViewComponent
        ExportView {
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
        return createTemporaryObject(exportViewComponent, testCase)
    }

    function init() {
        actions.browseCalls = 0

        settingsController.exportDefaultDirectory = "test:///export/default"
        settingsController.exportArchiveFormat = 0
        settingsController.exportIncludeFormulas = true

        exportController.currentMode = 0
        exportController.isPaused = false
        exportController.progress = 0
        exportController.phase = ""
        exportController.error = ""
        exportController.clearCalls = 0
        exportController.cancelCalls = 0
        exportController.togglePauseCalls = 0
        exportController.exportCalls = 0
        exportController.lastExportArgs = ({})
    }

    function test_EXP_002_clearButtonResetsExportState() {
        var view = createView()
        var clearButton = findRequired(view, "exportClearButton")

        clearButton.clicked()

        compare(exportController.clearCalls, 1)
    }

    function test_EXP_003_cancelButtonCallsController() {
        exportController.currentMode = 1
        var view = createView()
        var cancelButton = findRequired(view, "exportCancelButton")

        cancelButton.clicked()

        compare(exportController.cancelCalls, 1)
    }

    function test_EXP_004_pauseButtonCallsController() {
        exportController.currentMode = 1
        var view = createView()
        var pauseButton = findRequired(view, "exportTogglePauseButton")

        pauseButton.clicked()

        compare(exportController.togglePauseCalls, 1)
    }

    function test_EXP_005_startBuildsPayloadAndStartsExport() {
        var view = createView()
        var addButton = findRequired(view, "exportAddEntryButton")
        var startButton = findRequired(view, "exportStartButton")

        addButton.clicked()
        startButton.clicked()

        compare(exportController.exportCalls, 1)
        verify(String(exportController.lastExportArgs.payload).indexOf("Annual") !== -1)
        verify(String(exportController.lastExportArgs.path).indexOf("/export") !== -1)
    }

    function test_EXP_001_defaultDirectoryIsLoadedFromSettings() {
        settingsController.exportDefaultDirectory = "test:///export/from-settings"
        var view = createView()
        var formPanel = findRequired(view, "exportFormPanel")

        compare(formPanel.targetDirectory, "test:///export/from-settings")
    }

    function test_EXP_006_browseDirectoryRoutesToActions() {
        var view = createView()
        var browseButton = findRequired(view, "exportBrowseDirectoryButton")

        browseButton.clicked()

        compare(actions.browseCalls, 1)
    }

    function test_EXP_007_selectedDirectoryUpdatesFormTarget() {
        var view = createView()
        var formPanel = findRequired(view, "exportFormPanel")

        actions.exportDirectorySelected("test:///export/selected")

        compare(formPanel.targetDirectory, "test:///export/selected")
    }

    function test_EXP_008_settingsSyncUpdatesArchiveFormat() {
        var view = createView()
        var formPanel = findRequired(view, "exportFormPanel")

        settingsController.exportArchiveFormat = 1

        compare(formPanel.packageFormatIndex, 1)
    }
}
