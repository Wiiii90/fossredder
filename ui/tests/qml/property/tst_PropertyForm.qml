/**
 * @file ui/tests/qml/property/tst_PropertyForm.qml
 * @brief Provides QML tests for PropertyForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "PropertyFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedProperty: null
        property string selectedPropertyId: ""
        property var properties: []
        property var contracts: []

        function basicFormState(name, aliases, selectedIds) {
            var aliasValues = aliases || []
            return {
                name: name || "",
                aliases: aliasValues,
                aliasInputText: "",
                aliasIndex: aliasValues.length > 0 ? 0 : -1,
                selectedIds: selectedIds || []
            }
        }

        function normalizeStrings(values) {
            var out = []
            if (!values)
                return out
            for (var i = 0; i < values.length; ++i)
                out.push(String(values[i]))
            return out
        }

        function addUniqueTrimmed(values, value) {
            var out = values ? values.slice(0) : []
            var next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeAt(values, index) {
            var out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }

        function propertyRows() {
            return properties || []
        }

        function contractRows() {
            return contracts || []
        }

        function navigatedId(rows, currentId, delta, fallbackIndex) {
            if (!rows || rows.length === 0)
                return ""
            var index = -1
            for (var i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === String(currentId || "")) {
                    index = i
                    break
                }
            }
            if (index < 0)
                index = fallbackIndex
            else
                index = (index + delta + rows.length) % rows.length
            return String(rows[index].id || "")
        }

        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) {
            if (!rows || rows.length === 0)
                return ""
            var kept = []
            for (var i = 0; i < rows.length; ++i) {
                var rowId = String(rows[i][key] || "")
                if (rowId !== String(removedId || ""))
                    kept.push(rows[i])
            }
            if (kept.length === 0)
                return ""
            var idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1))
            return String(kept[idx][key] || "")
        }
    }

    property var propertyController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
        }

        function saveProperty(id, name, type, description, aliases) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                type: type,
                description: description,
                aliases: aliases
            }
            return id && id.length > 0 ? id : "property-new"
        }

        function deleteProperty(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var propertyController: testCase.propertyController
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 140
        property int viewAliasPanelPreferredHeight: 180
        property int spacingSmall: 6
        property int spacingLarge: 20
        property color textMuted: "#666666"
        property color selectionHighlight: "#dddddd"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int viewActionButtonWidth: 120
        property int radius: 3
    }

    Component {
        id: propertyFormComponent

        PropertyForm {
            width: 960
            height: 640
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createPropertyObject(source) {
        if (!source)
            return null
        var propertyObject = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: ""; property string name: ""; property var aliases: [] }', testCase)
        propertyObject.id = source.id || ""
        propertyObject.name = source.name || ""
        propertyObject.aliases = source.aliases || []
        return propertyObject
    }

    function createForm(selectedProperty) {
        var propertyObject = createPropertyObject(selectedProperty)
        session.selectedProperty = propertyObject
        session.selectedPropertyId = propertyObject ? propertyObject.id : ""
        return createTemporaryObject(propertyFormComponent, testCase)
    }

    function init() {
        propertyController.reset()
        session.selectedProperty = null
        session.selectedPropertyId = ""
        session.properties = []
        session.contracts = []
    }

    function test_createModeSavesPropertyAndSelectsNewId() {
        var form = createForm(null)
        var nameField = findRequired(form, "propertyNameField")
        var createButton = findRequired(form, "propertyCreateButton")

        nameField.text = "Flat 12"
        createButton.clicked()

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "")
        compare(propertyController.lastSave.name, "Flat 12")
        compare(session.selectedPropertyId, "property-new")
    }

    function test_aliasButtonsAddAndRemoveAlias() {
        var form = createForm(null)
        var aliasInput = findRequired(form, "propertyAliasInput")
        var addAliasButton = findRequired(form, "propertyAddAliasButton")
        var removeAliasButton = findRequired(form, "propertyRemoveAliasButton")

        aliasInput.text = "A1"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "A1")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_readModeLoadsSelectedPropertyState() {
        var form = createForm({ id: "property-3", name: "House", aliases: ["H1", "H2"] })
        var nameField = findRequired(form, "propertyNameField")

        compare(form.isEdit, true)
        compare(nameField.text, "House")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "H1")
        compare(form.aliases[1], "H2")
    }

    function test_updateModeSavesCurrentPropertyId() {
        var form = createForm({ id: "property-5", name: "Old", aliases: ["One"] })
        var nameField = findRequired(form, "propertyNameField")
        var updateButton = findRequired(form, "propertyUpdateButton")

        nameField.text = "New Property"
        updateButton.clicked()

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "property-5")
        compare(propertyController.lastSave.name, "New Property")
    }

    function test_contractSelectionUpdatesSelectedIds() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var checkBox = findRequired(form, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.clicked()

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_navigationButtonsMoveSelectionId() {
        session.properties = [
            { id: "property-1", name: "A" },
            { id: "property-2", name: "B" }
        ]
        var form = createForm({ id: "property-1", name: "A", aliases: [] })
        var nextButton = findRequired(form, "propertyNextButton")
        var previousButton = findRequired(form, "propertyPreviousButton")

        nextButton.clicked()
        compare(session.selectedPropertyId, "property-2")

        previousButton.clicked()
        compare(session.selectedPropertyId, "property-1")
    }

    function test_deleteButtonDeletesCurrentProperty() {
        var form = createForm({ id: "property-7", name: "Lot", aliases: [] })
        var deleteButton = findRequired(form, "propertyDeleteButton")

        deleteButton.clicked()

        compare(propertyController.deleteCalls, 1)
        compare(propertyController.lastDeleteId, "property-7")
    }
}
