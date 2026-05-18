/**
 * @file ui/tests/qml/export/tst_ExportSidebar.qml
 * @brief Provides QML tests for ExportSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var exportWorkflow: QtObject {
        property var runs: []
        property int openCalls: 0
        property int removeCalls: 0
        function openRunLocationAt(index) { openCalls += 1 }
        function removeRunAt(index) { removeCalls += 1 }
    }

    property var appContext: QtObject {
        property var exportWorkflow: testCase.exportWorkflow
    }

    property var theme: QtObject {
        property int spacingMedium: 8
        property int spacingSmall: 6
        property int borderWidthThin: 1
        property int radius: 3
        property int viewCompactActionButtonSize: 28
        property color accent: "#3366ff"
        property color border: "#cccccc"
        property color success: "#008800"
        property color warning: "#aa8800"
        property color danger: "#aa0000"
        property color textMuted: "#666666"
    }

    Component {
        id: exportSidebarComponent
        ExportSidebar {
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
        return createTemporaryObject(exportSidebarComponent, testCase)
    }

    function init() {
        exportWorkflow.runs = [
            { logId: "export-1", time: "2026-05-16 10:00:00", status: "Success", file: "/tmp/export.xlsx", message: "done", payload: "{\"items\":[]}" }
        ]
        exportWorkflow.openCalls = 0
        exportWorkflow.removeCalls = 0
    }

    function test_runListIsBoundToExportRuns() {
        var view = createView()
        var runList = findRequired(view, "runLogList")

        compare(runList.count, 1)
    }
}
