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
        property int dataRevision: 0
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

        function selectPropertyState(newId, newName, newAliases, newContractIds) {
            if (selectedProperty && selectedProperty.setState)
                selectedProperty.setState(newId, newName, newAliases, newContractIds)
            selectedPropertyId = String(newId || "")
        }
    }

    property var propertyController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var savePropertyOverride: null

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            savePropertyOverride = null
        }

        function defaultSaveProperty(id, name, aliases, contractIds) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            }
            return id && id.length > 0 ? id : "property-new"
        }

        function saveProperty(id, name, aliases, contractIds) {
            if (savePropertyOverride)
                return savePropertyOverride(id, name, aliases, contractIds)
            return defaultSaveProperty(id, name, aliases, contractIds)
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

        function deleteProperty(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var workspaceFacade: testCase.propertyController
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int spacing: 12
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 140
        property int viewAliasPanelPreferredHeight: 180
        property int viewAliasChipHeight: 30
        property int viewAliasChipRadius: 6
        property int viewCompactActionButtonSize: 40
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
        property int borderWidthThin: 1
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
        var propertyObject = Qt.createQmlObject('import QtQml 2.15; QtObject { signal changed(); property string id: ""; property string name: ""; property var aliases: []; property var contractIds: []; function setState(newId, newName, newAliases, newContractIds) { id = newId || ""; name = newName || ""; aliases = newAliases || []; contractIds = newContractIds || []; changed(); } }', testCase)
        propertyObject.setState(source.id || "", source.name || "", source.aliases || [], source.contractIds || [])
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
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var nameField = findRequired(form, "propertyNameField")
        var createButton = findRequired(form, "propertyCreateButton")
        var aliasInput = findRequired(form, "propertyAliasInput")
        var addAliasButton = findRequired(form, "propertyAddAliasButton")
        var checkBox = findRequired(form, "propertyContractCheckBox")

        nameField.text = "Flat 12"
        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        checkBox.checked = true
        checkBox.toggled()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        propertyController.savePropertyOverride = function(id, name, aliases, contractIds) {
            var result = propertyController.defaultSaveProperty(id, name, aliases, contractIds)
            session.selectedProperty = createPropertyObject({
                id: result,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            })
            session.selectedPropertyId = result
            session.dataRevision += 1
            return result
        }
        try {
            createButton.clicked()
        } finally {
            propertyController.savePropertyOverride = null
        }

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "")
        compare(propertyController.lastSave.name, "Flat 12")
        compare(propertyController.lastSave.aliases.length, 1)
        compare(propertyController.lastSave.aliases[0], "Alias One")
        compare(propertyController.lastSave.contractIds.length, 1)
        compare(propertyController.lastSave.contractIds[0], "contract-1")
        compare(session.selectedPropertyId, "property-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        session.selectedProperty = createPropertyObject({ id: "property-other", name: "Other", aliases: ["Other"], contractIds: [] })
        session.selectedPropertyId = "property-other"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedProperty = createPropertyObject({ id: "property-new", name: "Flat 12", aliases: ["Alias One"], contractIds: ["contract-1"] })
        session.selectedPropertyId = "property-new"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
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

    function test_editModeAliasButtonAddsAliasToFormState() {
        var form = createForm({ id: "property-4", name: "Flat", aliases: ["Base"] })
        var aliasInput = findRequired(form, "propertyAliasInput")
        var addAliasButton = findRequired(form, "propertyAddAliasButton")

        aliasInput.text = "Alias Two"
        compare(aliasInput.text, "Alias Two")
        compare(addAliasButton.enabled, true)
        compare(addAliasButton.width > 0, true)
        compare(addAliasButton.height > 0, true)
        addAliasButton.clicked()

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
    }

    function test_aliasPanelGetsLayoutSpaceForRenderedAliases() {
        var form = createForm({ id: "property-4", name: "Flat", aliases: ["Base"] })
        var aliasScroll = findRequired(form, "propertyAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
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

    function test_updateModeKeepsSelectedContractsAndSavesThem() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })
        var nameField = findRequired(form, "propertyNameField")
        var updateButton = findRequired(form, "propertyUpdateButton")
        var aliasInput = findRequired(form, "propertyAliasInput")
        var addAliasButton = findRequired(form, "propertyAddAliasButton")
        var checkBox = findRequired(form, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        nameField.text = "New Property"
        propertyController.savePropertyOverride = function(id, name, aliases, contractIds) {
            var result = propertyController.defaultSaveProperty(id, name, aliases, contractIds)
            if (session.selectedProperty && session.selectedProperty.setState)
                session.selectedProperty.setState(id || "property-6", name, aliases, contractIds)
            session.selectedPropertyId = id || "property-6"
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            propertyController.savePropertyOverride = null
        }

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "property-6")
        compare(propertyController.lastSave.name, "New Property")
        compare(propertyController.lastSave.aliases.length, 2)
        compare(propertyController.lastSave.aliases[0], "One")
        compare(propertyController.lastSave.aliases[1], "Alias Two")
        compare(propertyController.lastSave.contractIds.length, 1)
        compare(propertyController.lastSave.contractIds[0], "contract-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectedProperty = createPropertyObject({ id: "property-7", name: "Other", aliases: ["Other"], contractIds: [] })
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedProperty = createPropertyObject({
            id: "property-6",
            name: "New Property",
            aliases: ["Property Alias", "Property Fresh Alias"],
            contractIds: ["contract-1"]
        })
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Property Alias")
        compare(form.aliases[1], "Property Fresh Alias")

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        var form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectPropertyState("property-6", "Old", ["One", "Alias Two"], [])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectPropertyState("property-7", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectPropertyState("property-6", "Old", ["One", "Alias Two"], [])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
    }

    function test_dataRevisionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        var form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectedProperty.aliases = ["One", "Alias Two"]
        session.selectedProperty.contractIds = ["contract-1"]
        session.dataRevision += 1

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_contractSelectionUpdatesSelectedIds() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var checkBox = findRequired(form, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        var updateButton = findRequired(form, "propertyUpdateButton")
        var nameField = findRequired(form, "propertyNameField")
        nameField.text = "Flat 12"
        updateButton.clicked()
        compare(propertyController.lastSave.contractIds.length, 1)
        compare(propertyController.lastSave.contractIds[0], "contract-1")
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

    function test_createShortcutButtonClearsSelectionAndSwitchesToCreateMode() {
        var form = createForm({ id: "property-9", name: "Selected", aliases: [] })
        var createModeButton = findRequired(form, "propertyCreateModeButton")

        createModeButton.clicked()

        compare(session.selectedPropertyId, "")
        compare(form.isEdit, false)
    }

    function test_deleteButtonDeletesCurrentProperty() {
        var form = createForm({ id: "property-7", name: "Lot", aliases: [] })
        var deleteButton = findRequired(form, "propertyDeleteButton")

        deleteButton.clicked()

        compare(propertyController.deleteCalls, 1)
        compare(propertyController.lastDeleteId, "property-7")
    }
}
