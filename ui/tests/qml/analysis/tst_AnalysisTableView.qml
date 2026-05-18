/**
 * @file ui/tests/qml/analysis/tst_AnalysisTableView.qml
 * @brief Provides QML tests for AnalysisTableView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisTableViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var lastAnalysisResult: ({
            transactions: [
                { id: "tx-1", amount: 100.0, contractType: "lease", propertyIds: ["property-1"], propertyNames: ["Lot"] },
                { id: "tx-2", amount: 50.0, contractType: "service", propertyIds: ["property-2"], propertyNames: ["House"] }
            ]
        })
    }

    property var appContext: QtObject {
        property var session: testCase.session
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color borderLight: "#d7d7d7"
        property color borderStrong: "#888888"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color chartText: "#000000"
        property var analysis: QtObject {
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
        }
    }

    Component {
        id: tableViewComponent
        AnalysisTableView {
            width: 960
            height: 640
            appContext: testCase.appContext
            theme: testCase.theme
            adjustmentAmountsById: ({})
        }
    }

    function createView() {
        return createTemporaryObject(tableViewComponent, testCase)
    }

    function init() {
        session.lastAnalysisResult = ({
            transactions: [
                { id: "tx-1", amount: 100.0, contractType: "lease", propertyIds: ["property-1"], propertyNames: ["Lot"] },
                { id: "tx-2", amount: 50.0, contractType: "service", propertyIds: ["property-2"], propertyNames: ["House"] }
            ]
        })
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function test_rebuildMatrixProducesTotals() {
        var view = createView()

        compare(view.contractTypes.length, 2)
        compare(view.matrixPropertyNames.length, 2)
        compare(view.grandTotal, 150.0)
        compare(view.propertyTotal("Lot"), 100.0)
        compare(view.propertyTotal("House"), 50.0)
    }

    function test_tablePreviewHasRenderableGeometry() {
        var view = createView()
        var viewport = findRequired(view, "analysisTableViewport")

        verify(view.width > 0)
        verify(view.height > 0)
        verify(viewport.width > 0)
        verify(viewport.height > 0)
    }

    function test_rebuildMatrixAcceptsCoreTableRowsWhenTransactionsAreAbsent() {
        session.lastAnalysisResult = ({
            type: "tab",
            table: [
                ["2026-01-01", "Rent", "100.0"],
                ["2026-01-02", "Service", "50.0"]
            ]
        })

        var view = createView()

        compare(view.contractTypes.length, 1)
        compare(view.matrixPropertyNames.length, 1)
        compare(view.grandTotal, 150.0)
        compare(view.propertyTotal("Unassigned"), 150.0)
    }

    function test_rebuildMatrixUsesResultPropertyNames() {
        session.lastAnalysisResult = ({
            transactions: [
                {
                    id: "tx-1",
                    amount: 100.0,
                    contractType: "lease",
                    propertyIds: ["property-hex-like"],
                    propertyNames: ["Readable Property"]
                }
            ]
        })

        var view = createView()

        compare(view.matrixPropertyNames.length, 1)
        compare(view.matrixPropertyNames[0], "Readable Property")
        compare(view.propertyTotal("Readable Property"), 100.0)
    }
}
