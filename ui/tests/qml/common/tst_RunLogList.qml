/**
 * @file ui/tests/qml/common/tst_RunLogList.qml
 * @brief Provides QML tests for RunLogList behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

import "../Lookup.js" as Lookup

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
                    payload: '{"items":[{"objectType":"Annual"},{"objectType":"Analysis","exportType":"CSV"}]}',
                    draftAttached: false,
                    draftId: "",
                    statementId: "statement-1"
                }
            ]
        }
    }

    function createControl() {
        return createTemporaryObject(runLogListComponent, testCase)
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
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

    function test_CTRL_RL_005_delegateHasClickableHeight() {
        var control = createControl()
        wait(0)

        var mouseArea = findRequired(control, "runLogRow_log-1")
        verify(mouseArea.height > 0)
    }

    function test_CTRL_RL_006_deletedRunWithoutStatementIsNotClickable() {
        var control = createTemporaryObject(runLogListComponent, testCase, {
            model: [
                {
                    logId: "deleted-log",
                    time: "2026-01-01T10:00:00Z",
                    status: "Deleted",
                    file: "test:///imports/a.pdf",
                    message: "Imported statement was deleted.",
                    payload: "",
                    draftAttached: false,
                    draftId: "",
                    statementId: ""
                }
            ]
        })
        wait(0)

        var mouseArea = findRequired(control, "runLogRow_deleted-log")
        verify(!mouseArea.enabled)
    }

    function test_CTRL_RL_004_clickEmitsDraftId() {
        var control = createTemporaryObject(runLogListComponent, testCase, {
            model: [
                {
                    logId: "draft-log",
                    time: "2026-01-01T10:00:00Z",
                    status: "Draft",
                    file: "test:///imports/a.pdf",
                    message: "draft",
                    payload: "",
                    draftAttached: true,
                    draftId: "draft-1",
                    statementId: ""
                }
            ]
        })
        var received = ({})
        control.runClicked.connect(function(index, logId, draftAttached, statementId, draftId) {
            received = { index: index, logId: logId, draftAttached: draftAttached, statementId: statementId, draftId: draftId }
        })
        wait(0)

        var mouseArea = findRequired(control, "runLogRow_draft-log")
        verify(mouseArea.enabled)
        mouseArea.activateRun()

        compare(received.logId, "draft-log")
        compare(received.draftAttached, true)
        compare(received.draftId, "draft-1")
    }

    function test_CTRL_RL_002_deleteClickEmitsDeleteSignal() {
        var control = createControl()
        var received = ({})
        control.deleteClicked.connect(function(index, draftAttached, draftId) {
            received = { index: index, draftAttached: draftAttached, draftId: draftId }
        })
        wait(0)

        var deleteButton = findRequired(control, "runLogDelete_log-1")
        deleteButton.clicked()

        compare(received.index, 0)
        compare(received.draftAttached, false)
    }

}
