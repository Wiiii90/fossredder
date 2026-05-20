/**
 * @file ui/tests/qml/property/tst_PropertyView.qml
 * @brief Provides QML tests for PropertyView behavior.
 */

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "PropertyViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property int dataRevision: 0
        property string selectedPropertyId: ""
        property var selectedProperty: null
        property var propertyRowsData: []
        property var contracts: []
        function propertyRows() { return propertyRowsData || [] }
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
            function saveProperty(id, name, aliases, contractIds) { return id || "property-new" }
            function deleteProperty(id) {}
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
        id: propertyViewComponent
        PropertyView { width: 960; height: 640; appContext: testCase.appContext; theme: testCase.theme }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createView() {
        return createTemporaryObject(propertyViewComponent, testCase)
    }

    function init() {
        session.dataRevision = 0
        session.selectedPropertyId = ""
        session.selectedProperty = null
        session.propertyRowsData = [
            { id: "property-1", name: "P1" },
            { id: "property-2", name: "P2" },
            { id: "property-3", name: "P3" }
        ]
        session.contracts = []
    }

    function test_navigationStaysEnabledWithSingleRow() {
        session.propertyRowsData = [
            { id: "property-1", name: "P1" }
        ]
        var view = createView()
        var nextButton = findRequired(view, "propertyNextButton")
        var previousButton = findRequired(view, "propertyPreviousButton")

        compare(nextButton.enabled, true)
        compare(previousButton.enabled, true)

        nextButton.clicked()
        compare(session.selectedPropertyId, "property-1")
    }

    function test_navigationCyclesThroughCreateMode() {
        session.selectedPropertyId = "property-3"
        session.selectedProperty = { id: "property-3", name: "P3" }
        var view = createView()

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "")

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "property-1")

        session.selectedPropertyId = "property-1"
        session.selectedProperty = { id: "property-1", name: "P1" }
        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "")

        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "property-3")
    }

    function test_createModeNavigationStartsAtEdges() {
        var view = createView()

        findRequired(view, "propertyNextButton").clicked()
        compare(session.selectedPropertyId, "property-1")

        session.selectedPropertyId = ""
        findRequired(view, "propertyPreviousButton").clicked()
        compare(session.selectedPropertyId, "property-3")
    }
}
