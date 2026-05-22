/**
 * @file ui/tests/qml/booking/tst_BookingStatementsSidebar.qml
 * @brief Provides QML tests for BookingStatementsSidebar behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingStatementsSidebarTests"
    when: windowShown
    width: 960
    height: 640

    property var statementRowsData: []
    property var transactionRowsByStatementId: ({})

    property var session: QtObject {
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
        property int dataRevision: 0

        function statementRows() {
            return testCase.statementRowsData || []
        }

        function statementTransactionRows(statementId) {
            return testCase.transactionRowsByStatementId[String(statementId || "")] || []
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int margins: 8
        property int spacingLarge: 20
        property int viewCompactActionButtonSize: 28
        property int borderWidthThin: 1
        property int radius: 3
        property color selectionHighlight: "#aaccee"
        property color borderSoft: "#cccccc"
        property color textMuted: "#666666"
        property color textPrimary: "#000000"
    }

    Component {
        id: sidebarComponent
        BookingStatementsSidebar {
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
        return createTemporaryObject(sidebarComponent, testCase)
    }

    function init() {
        statementRowsData = []
        transactionRowsByStatementId = ({})
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
        session.dataRevision = 0
    }

    function test_transactionClickSelectsTransactionRow() {
        statementRowsData = [
            { id: "statement-1", name: "S1" }
        ]
        transactionRowsByStatementId["statement-1"] = [
            { id: "tx-1", name: "Tx 1", bookingDate: "2026-01-01" }
        ]

        var view = createView()
        var txMouse = findRequired(view, "bookingTransactionMouse_tx-1")

        txMouse.triggerClick()

        compare(session.selectedStatementId, "statement-1")
        compare(session.selectedTransactionId, "tx-1")
    }

    function test_statementClickClearsTransactionSelection() {
        statementRowsData = [
            { id: "statement-1", name: "S1" }
        ]
        transactionRowsByStatementId["statement-1"] = [
            { id: "tx-1", name: "Tx 1", bookingDate: "2026-01-01" }
        ]
        session.selectedStatementId = "statement-1"
        session.selectedTransactionId = "tx-1"

        var view = createView()
        var statementMouse = findRequired(view, "bookingStatementMouse_statement-1")

        statementMouse.triggerClick()

        compare(session.selectedStatementId, "statement-1")
        compare(session.selectedTransactionId, "")
    }

    function test_statementSelectionScrollsStatementToTop() {
        for (var i = 0; i < 12; ++i)
            statementRowsData.push({ id: "statement-" + i, name: "S" + i })

        var view = createView()
        view.height = 120
        var flick = findRequired(view, "bookingStatementsFlick")
        var row = findRequired(view, "bookingStatementRow_statement-8")

        var statementMouse = findRequired(view, "bookingStatementMouse_statement-8")
        statementMouse.triggerClick()
        wait(0)

        var expectedY = row.mapToItem(flick.contentItem, 0, 0).y
        compare(Math.round(flick.contentY), Math.round(expectedY))
    }

    function test_initialStatementSelectionScrollsStatementToTop() {
        for (var i = 0; i < 12; ++i)
            statementRowsData.push({ id: "statement-" + i, name: "S" + i })
        session.selectedStatementId = "statement-8"

        var view = createView()
        view.height = 120
        wait(0)
        var flick = findRequired(view, "bookingStatementsFlick")
        var row = findRequired(view, "bookingStatementRow_statement-8")

        var expectedY = row.mapToItem(flick.contentItem, 0, 0).y
        compare(Math.round(flick.contentY), Math.round(expectedY))
    }

    function test_initialTransactionSelectionScrollsTransactionToTop() {
        statementRowsData = [
            { id: "statement-1", name: "S1" },
            { id: "statement-2", name: "S2" }
        ]
        transactionRowsByStatementId["statement-1"] = []
        for (var i = 0; i < 10; ++i)
            transactionRowsByStatementId["statement-1"].push({ id: "tx-" + i, name: "Tx " + i, bookingDate: "2026-01-01" })
        session.selectedStatementId = "statement-1"
        session.selectedTransactionId = "tx-7"

        var view = createView()
        view.height = 120
        wait(0)
        var flick = findRequired(view, "bookingStatementsFlick")
        var row = findRequired(view, "bookingTransactionRow_tx-7")

        var expectedY = row.mapToItem(flick.contentItem, 0, 0).y
        compare(Math.round(flick.contentY), Math.round(expectedY))
    }

    function test_transactionSelectionScrollsTransactionToTop() {
        statementRowsData = [
            { id: "statement-1", name: "S1" },
            { id: "statement-2", name: "S2" }
        ]
        transactionRowsByStatementId["statement-1"] = []
        for (var i = 0; i < 10; ++i)
            transactionRowsByStatementId["statement-1"].push({ id: "tx-" + i, name: "Tx " + i, bookingDate: "2026-01-01" })

        var view = createView()
        view.height = 120
        var flick = findRequired(view, "bookingStatementsFlick")
        var row = findRequired(view, "bookingTransactionRow_tx-7")

        var txMouse = findRequired(view, "bookingTransactionMouse_tx-7")
        txMouse.triggerClick()
        wait(0)

        var expectedY = row.mapToItem(flick.contentItem, 0, 0).y
        compare(Math.round(flick.contentY), Math.round(expectedY))
    }
}
