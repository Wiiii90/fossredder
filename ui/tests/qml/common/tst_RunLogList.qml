/**
 * @file ui/tests/qml/common/tst_RunLogList.qml
 * @brief Provides QML tests for RunLogList behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

TestCase {
    id: testCase
    name: "RunLogListTests"
    when: windowShown
    width: 720
    height: 480

    property var theme: QtObject {
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
        id: runLogListComponent
        RunLogList {
            width: 700
            height: 420
            theme: testCase.theme
            model: [
                {
                    logId: "log-1",
                    time: "2026-01-01T10:00:00Z",
                    status: "Success",
                    file: "test:///exports/a.xlsx",
                    message: "done",
                    payload: '{"items":[{"objectType":"Annual"},{"objectType":"Analysis","exportType":"CSV"}]}'
                }
            ]
        }
    }

    function createControl() {
        return createTemporaryObject(runLogListComponent, testCase)
    }

    function test_CTRL_RL_003_payloadSummaryParsesAnnualAndAnalysisCounts() {
        var control = createControl()
        var summary = control.payloadSummary('{"items":[{"objectType":"Annual"},{"objectType":"Analysis","exportType":"XLSX"}]}')

        verify(summary.indexOf("Annuals") !== -1)
        verify(summary.indexOf("Analyses") !== -1)
    }

    function test_CTRL_RL_002_fileNameReturnsLastSegment() {
        var control = createControl()
        compare(control.fileName("test:///tmp/export.xlsx"), "export.xlsx")
    }

    function test_CTRL_RL_001_modelIsAssigned() {
        var control = createControl()
        verify(control.model !== null)
        compare(control.model.length, 1)
    }

}
