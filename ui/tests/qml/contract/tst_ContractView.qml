/**
 * @file ui/tests/qml/contract/tst_ContractView.qml
 * @brief Provides QML tests for ContractView behavior.
 */

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ContractViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedContractId: ""
        property var selectedContract: null
        property var contractRowsData: []
        property var actors: []
        property var properties: []
        function contractRows() { return contractRowsData || [] }
        function actorRows() { return actors || [] }
        function propertyRows() { return properties || [] }
        function indexOfId(rows, id) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return i
            }
            return -1
        }
        function contractFormState(name, type, actorIds, propertyIds, aliases) {
            return { name: name || "", type: type || "", selectedActorIds: actorIds || [], selectedPropertyIds: propertyIds || [], aliases: aliases || [], aliasInputText: "", aliasIndex: -1 }
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var sessionState: testCase.session
        property var workspaceFacade: QtObject {
            function addUniqueTrimmed(values, value) { return values || [] }
            function removeAt(values, index) { return values || [] }
            function saveContract(id, name, type, actorIds, propertyIds, aliases) { return id || "contract-new" }
            function deleteContract(id) {}
        }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int controlHeight: 32
        property int formFieldWidth: 220
        property int viewCompactActionButtonSize: 28
        property int viewActionButtonWidth: 120
        property int viewNavigationButtonWidth: 40
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 60
        property int viewAliasPanelPreferredHeight: 90
        property int viewAliasChipHeight: 24
        property int viewAliasChipRadius: 4
        property int viewSelectionPanelMinHeight: 80
        property int viewSelectionPanelPreferredHeight: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color selectionHighlight: "#aaccee"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color borderSoft: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: contractViewComponent
        ContractView { width: 960; height: 640; appContext: testCase.appContext; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(contractViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedContractId = ""
        session.selectedContract = null
        session.contractRowsData = [
            { id: "contract-1", name: "C1" },
            { id: "contract-2", name: "C2" },
            { id: "contract-3", name: "C3" }
        ]
        session.actors = []
        session.properties = []
    }

    function test_navigationStaysEnabledWithSingleRow() {
        session.contractRowsData = [
            { id: "contract-1", name: "C1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "contractNextButton")
        var previousButton = findRequired(view, "contractPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedContractId, "contract-1")
    }

    function test_navigationCyclesThroughCreateMode() {
        session.selectedContractId = "contract-3"
        session.selectedContract = { id: "contract-3", name: "C3" }
        var view = createView()

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "")

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "contract-1")

        session.selectedContractId = "contract-1"
        session.selectedContract = { id: "contract-1", name: "C1" }
        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "")

        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "contract-3")
    }

    function test_createModeNavigationStartsAtEdges() {
        var view = createView()

        findRequired(view, "contractNextButton").clicked()
        compare(session.selectedContractId, "contract-1")

        session.selectedContractId = ""
        findRequired(view, "contractPreviousButton").clicked()
        compare(session.selectedContractId, "contract-3")
    }
}
