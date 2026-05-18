/**
 * @file ui/tests/qml/export/tst_ExportProgressBar.qml
 * @brief Provides QML tests for ExportProgressBar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportProgressBarTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property color danger: "#aa0000"
        property color textPrimary: "#000000"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property int radius: 3
    }

    property var exportWorkflow: QtObject {
        property int currentMode: 0
        property real progress: 0.0
        property string phase: ""
        property string error: ""
    }

    Component {
        id: exportProgressBarComponent
        ExportProgressBar {
            width: 700
            theme: testCase.theme
            exportWorkflow: testCase.exportWorkflow
            hasExportCtrl: true
            readyText: "Ready"
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(exportProgressBarComponent, testCase)
    }

    function init() {
        exportWorkflow.currentMode = 0
        exportWorkflow.progress = 0.0
        exportWorkflow.phase = ""
        exportWorkflow.error = ""
    }

    function test_EXP_004_progressUsesControllerWhenRunning() {
        exportWorkflow.currentMode = 1
        exportWorkflow.progress = 0.65

        var view = createView()
        var bar = findRequired(view, "exportProgressBar")

        compare(bar.value, 0.65)
    }

    function test_EXP_005_statusShowsPhaseBeforeReady() {
        exportWorkflow.currentMode = 1
        exportWorkflow.phase = "Exporting"

        var view = createView()
        var statusLabel = findRequired(view, "exportProgressStatusLabel")

        compare(statusLabel.text, "Exporting")
    }

    function test_EXP_003_statusShowsErrorWhenPresent() {
        exportWorkflow.currentMode = 1
        exportWorkflow.phase = "Exporting"
        exportWorkflow.error = "Disk full"

        var view = createView()
        var statusLabel = findRequired(view, "exportProgressStatusLabel")

        compare(statusLabel.text, "Disk full")
    }

}
