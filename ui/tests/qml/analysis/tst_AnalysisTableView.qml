/**
 * @file ui/tests/qml/analysis/tst_AnalysisTableView.qml
 * @brief Provides QML tests for AnalysisTableView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

TestCase {
    id: testCase
    name: "AnalysisTableViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var lastAnalysisResult: ({
            transactions: [
                { id: "tx-1", amount: 100.0, contractType: "lease", propertyIds: ["property-1"] },
                { id: "tx-2", amount: 50.0, contractType: "service", propertyIds: ["property-2"] }
            ]
        })
        function propertyName(id) {
            if (id === "property-1")
                return "Lot"
            if (id === "property-2")
                return "House"
            return id
        }
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

    function test_rebuildMatrixProducesTotals() {
        var view = createView()

        compare(view.contractTypes.length, 2)
        compare(view.matrixPropertyNames.length, 2)
        compare(view.grandTotal, 150.0)
        compare(view.propertyTotal("Lot"), 100.0)
        compare(view.propertyTotal("House"), 50.0)
    }
}
