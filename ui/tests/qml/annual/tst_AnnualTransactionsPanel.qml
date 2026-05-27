/**
 * @file ui/tests/qml/annual/tst_AnnualTransactionsPanel.qml
 * @brief Provides QML tests for AnnualTransactionsPanel category rendering.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnnualTransactionsPanelTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color success: "#0a7f2e"
        property color warning: "#a86d00"
        property color danger: "#b0302f"
        property color info: "#1a73b8"
    }

    Component {
        id: panelComponent
        AnnualTransactionsPanel {
            width: 960
            height: 640
            theme: testCase.theme
            groupedTransactions: ({
                deduplicated: [{ id: "d-1", name: "Dedup", bookingDate: "2026-01-01", amount: 10.0, status: 0, allocatable: true }],
                similar: [{ id: "s-1", name: "Similar", bookingDate: "2026-01-02", amount: 11.0, status: 1, allocatable: true }],
                divergent: [{ id: "v-1", name: "Divergent", bookingDate: "2026-01-03", amount: 12.0, status: 2, allocatable: false, isMissingLive: true }],
                workspaceOnly: [{ id: "w-1", name: "WorkspaceOnly", bookingDate: "2026-01-04", amount: 13.0, status: 3, allocatable: true }]
            })
        }
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function test_ANN_T_001_rendersAllCategoryTogglesWhenRowsExist() {
        var panel = createPanel()
        wait(0)

        verify(findRequired(panel, "annualTransactionsSectionToggle_deduplicated") !== null)
        verify(findRequired(panel, "annualTransactionsSectionToggle_similar") !== null)
        verify(findRequired(panel, "annualTransactionsSectionToggle_divergent") !== null)
        verify(findRequired(panel, "annualTransactionsSectionToggle_workspaceOnly") !== null)
        verify(findRequired(panel, "annualTransactionsSectionToggle_missingLive") !== null)
    }
}
