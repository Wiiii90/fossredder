/**
 * @file ui/tests/qml/contract/tst_ContractActorsPanel.qml
 * @brief Provides QML tests for ContractActorsPanel behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup
import "../TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractActorsPanelTests"
    when: windowShown
    width: 960
    height: 320

    property var sessionState: QtObject {
        function displayRowsWithEmpty(rows, emptyLabel, textKey) {
            var out = [{ id: "", display: emptyLabel }]
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                out.push({ id: String(list[i].id || ""), display: String(list[i][textKey] || "") })
            }
            return out
        }
        function indexOfId(rows, targetId) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(targetId || ""))
                    return i
            }
            return -1
        }
    }

    property var actorRows: [
        { id: "actor-1", name: "Alice" },
        { id: "actor-2", name: "Bob" }
    ]

    property var contractState: QtObject {
        property var selectedActorIds: []
        function selectPrimaryActor(actorId) {
            var id = String(actorId || "").trim()
            selectedActorIds = id.length > 0 ? [id] : []
        }
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
    }

    Component {
        id: panelComponent
        ContractActorsPanel {
            width: 960
            height: 120
            theme: testCase.theme
            sessionState: testCase.sessionState
            contractState: testCase.contractState
            actorRows: testCase.actorRows
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createPanel() {
        return createTemporaryObject(panelComponent, testCase)
    }

    function init() {
        contractState.selectedActorIds = []
    }

    function test_CON_AP_001_dropdownSelectionWritesSelectedActorId() {
        var panel = createPanel()
        var comboBox = findRequired(panel, "contractActorComboBox")

        comboBox.currentIndex = 1
        comboBox.activated(1)

        compare(contractState.selectedActorIds.length, 1)
        compare(contractState.selectedActorIds[0], "actor-1")
    }

    function test_CON_AP_002_existingSelectionIsRendered() {
        contractState.selectedActorIds = ["actor-2"]
        var panel = createPanel()
        var comboBox = findRequired(panel, "contractActorComboBox")

        compare(comboBox.currentIndex > 0, true)
    }
}
