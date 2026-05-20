/**
 * @file ui/tests/qml/actor/tst_ActorView.qml
 * @brief Provides QML tests for ActorView behavior.
 */

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedActorId: ""
        property var selectedActor: null
        property var actorRowsData: []
        property var contracts: []
        function actorRows() { return actorRowsData || [] }
        function contractRows() { return contracts || [] }
        function indexOfId(rows, id) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(id || ""))
                    return i
            }
            return -1
        }
        function basicFormState(name, aliases, selectedIds) {
            return { name: name || "", aliases: aliases || [], selectedIds: selectedIds || [], aliasInputText: "", aliasIndex: -1 }
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var sessionState: testCase.session
        property var workspaceFacade: QtObject {
            function addUniqueTrimmed(values, value) { return values || [] }
            function removeAt(values, index) { return values || [] }
            function saveActor(id, name, aliases, contractIds) { return id || "actor-new" }
            function deleteActor(id) {}
        }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
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
        id: actorViewComponent
        ActorView { width: 960; height: 640; appContext: testCase.appContext; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(actorViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedActorId = ""
        session.selectedActor = null
        session.actorRowsData = [
            { id: "actor-1", name: "A1" },
            { id: "actor-2", name: "A2" },
            { id: "actor-3", name: "A3" }
        ]
        session.contracts = []
    }

    function test_navigationStaysEnabledWithSingleRow() {
        session.actorRowsData = [
            { id: "actor-1", name: "A1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "actorNextButton")
        var previousButton = findRequired(view, "actorPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedActorId, "actor-1")
    }

    function test_navigationCyclesThroughCreateMode() {
        session.selectedActorId = "actor-3"
        session.selectedActor = { id: "actor-3", name: "A3" }
        var view = createView()

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "")

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "actor-1")

        session.selectedActorId = "actor-1"
        session.selectedActor = { id: "actor-1", name: "A1" }
        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "")

        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "actor-3")
    }

    function test_createModeNavigationStartsAtEdges() {
        var view = createView()

        findRequired(view, "actorNextButton").clicked()
        compare(session.selectedActorId, "actor-1")

        session.selectedActorId = ""
        findRequired(view, "actorPreviousButton").clicked()
        compare(session.selectedActorId, "actor-3")
    }
}
