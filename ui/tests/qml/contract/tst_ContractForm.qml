/**
 * @file ui/tests/qml/contract/tst_ContractForm.qml
 * @brief Provides QML tests for ContractForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ContractFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedContract: null
        property string selectedContractId: ""
        property int dataRevision: 0
        property var contracts: []
        property var actors: []
        property var properties: []

        function contractFormState(name, type, selectedActorIds, selectedPropertyIds, aliases) {
            var aliasValues = aliases || []
            return {
                name: name || "",
                type: type || "",
                selectedActorIds: selectedActorIds || [],
                selectedPropertyIds: selectedPropertyIds || [],
                aliases: aliasValues,
                aliasInputText: "",
                aliasIndex: aliasValues.length > 0 ? 0 : -1
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

        function actorRows() {
            return actors || []
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

        function displayRowsWithEmpty(rows, emptyLabel, textKey) {
            var out = [{ id: "", display: emptyLabel }]
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                out.push({
                    id: String(list[i].id || ""),
                    display: String(list[i][textKey] || "")
                })
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

        function selectContractState(newId, newName, newType, newActorIds, newPropertyIds, newAliases) {
            if (selectedContract && selectedContract.setState)
                selectedContract.setState(newId, newName, newType, newActorIds, newPropertyIds, newAliases)
            selectedContractId = String(newId || "")
        }
    }

    property var contractController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var saveContractOverride: null

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            saveContractOverride = null
        }

        function defaultSaveContract(id, name, type, actorIds, propertyIds, aliases) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                type: type,
                actorIds: actorIds,
                propertyIds: propertyIds,
                aliases: aliases
            }
            return id && id.length > 0 ? id : "contract-new"
        }

        function saveContract(id, name, type, actorIds, propertyIds, aliases) {
            if (saveContractOverride)
                return saveContractOverride(id, name, type, actorIds, propertyIds, aliases)
            return defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
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

        function deleteContract(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var workspaceFacade: testCase.contractController
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 140
        property int viewAliasPanelPreferredHeight: 180
        property int viewAliasChipHeight: 24
        property int viewAliasChipRadius: 3
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int spacing: 8
        property int borderWidthThin: 1
        property color textMuted: "#666666"
        property color selectionHighlight: "#dddddd"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property int radius: 3
    }

    Component {
        id: contractFormComponent

        ContractForm {
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

    function createContractObject(source) {
        if (!source)
            return null
        var contractObject = Qt.createQmlObject('import QtQml 2.15; QtObject { signal changed(); property string id: ""; property string name: ""; property string type: ""; property var actorIds: []; property var propertyIds: []; property var aliases: []; function setState(newId, newName, newType, newActorIds, newPropertyIds, newAliases) { id = newId || ""; name = newName || ""; type = newType || ""; actorIds = newActorIds || []; propertyIds = newPropertyIds || []; aliases = newAliases || []; changed(); } }', testCase)
        contractObject.setState(source.id || "", source.name || "", source.type || "", source.actorIds || [], source.propertyIds || [], source.aliases || [])
        return contractObject
    }

    function createForm(selectedContract) {
        var contractObject = createContractObject(selectedContract)
        session.selectedContract = contractObject
        session.selectedContractId = contractObject ? contractObject.id : ""
        return createTemporaryObject(contractFormComponent, testCase)
    }

    function init() {
        contractController.reset()
        session.selectedContract = null
        session.selectedContractId = ""
        session.contracts = []
        session.actors = []
        session.properties = []
    }

    function test_createModeSavesContractAndSelectsNewId() {
        session.actors = [{ id: "actor-1", name: "Alice" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
        var form = createForm(null)
        var nameField = findRequired(form, "contractNameField")
        var createButton = findRequired(form, "contractCreateButton")
        var actorCombo = findRequired(form, "contractActorComboBox")
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")

        compare(createButton.enabled, false)
        nameField.text = "Lease 2026"
        compare(createButton.enabled, false)
        var typeField = findRequired(form, "contractTypeField")
        typeField.text = "lease"
        typeField.textEdited()
        compare(createButton.enabled, false)

        actorCombo.currentIndex = 1
        actorCombo.activated(1)
        propertyCheckBox.checked = true
        propertyCheckBox.toggled()
        aliasInput.text = "Main"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
        compare(createButton.enabled, true)

        contractController.saveContractOverride = function(id, name, type, actorIds, propertyIds, aliases) {
            var result = contractController.defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
            session.selectedContract = createContractObject({
                id: result,
                name: name,
                type: type,
                actorIds: actorIds,
                propertyIds: propertyIds,
                aliases: aliases
            })
            session.selectedContractId = result
            session.dataRevision += 1
            return result
        }
        try {
            createButton.clicked()
        } finally {
            contractController.saveContractOverride = null
        }

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "")
        compare(contractController.lastSave.name, "Lease 2026")
        compare(contractController.lastSave.aliases.length, 1)
        compare(contractController.lastSave.aliases[0], "Main")
        compare(contractController.lastSave.actorIds.length, 1)
        compare(contractController.lastSave.actorIds[0], "actor-1")
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
        compare(session.selectedContractId, "contract-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")

        session.selectedContract = createContractObject({
            id: "contract-other",
            name: "Other",
            type: "legacy",
            actorIds: [],
            propertyIds: [],
            aliases: ["Other"]
        })
        session.selectedContractId = "contract-other"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedContract = createContractObject({
            id: "contract-new",
            name: "Lease 2026",
            type: "lease",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["Main"]
        })
        session.selectedContractId = "contract-new"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_aliasButtonsAddAndRemoveAlias() {
        var form = createForm(null)
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")
        var removeAliasButton = findRequired(form, "contractRemoveAliasButton")

        aliasInput.text = "Main"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_editModeAliasButtonAddsAliasToFormState() {
        var form = createForm({ id: "contract-4", name: "Lease", type: "core", actorIds: ["actor-1"], propertyIds: [], aliases: ["Base"] })
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")

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
        var form = createForm({ id: "contract-4", name: "Lease", type: "core", actorIds: ["actor-1"], propertyIds: [], aliases: ["Base"] })
        var aliasScroll = findRequired(form, "contractAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
    }

    function test_readModeLoadsSelectedContractState() {
        var form = createForm({
            id: "contract-3",
            name: "Lease",
            type: "core",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["L1", "L2"]
        })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")

        compare(form.isEdit, true)
        compare(nameField.text, "Lease")
        compare(typeField.text, "core")
        compare(form.aliases.length, 2)
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedPropertyIds.length, 1)
    }

    function test_updateModeSavesCurrentContractIdWithType() {
        var form = createForm({ id: "contract-5", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: [] })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var updateButton = findRequired(form, "contractUpdateButton")

        nameField.text = "New Contract"
        typeField.text = "modern"
        typeField.textEdited()
        updateButton.clicked()

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "contract-5")
        compare(contractController.lastSave.name, "New Contract")
        compare(contractController.lastSave.type, "modern")
    }

    function test_updateModeKeepsSelectedRelationsAndSavesThem() {
        session.actors = [
            { id: "actor-1", name: "Alice" }
        ]
        session.properties = [
            { id: "property-1", name: "Lot" }
        ]
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var updateButton = findRequired(form, "contractUpdateButton")
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")
        var actorCombo = findRequired(form, "contractActorComboBox")
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")

        actorCombo.currentIndex = 1
        actorCombo.activated(1)
        propertyCheckBox.checked = true
        propertyCheckBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        nameField.text = "New Contract"
        typeField.text = "modern"
        typeField.textEdited()
        contractController.saveContractOverride = function(id, name, type, actorIds, propertyIds, aliases) {
            var result = contractController.defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
            if (session.selectedContract && session.selectedContract.setState)
                session.selectedContract.setState(id || "contract-6", name, type, actorIds, propertyIds, aliases)
            session.selectedContractId = id || "contract-6"
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            contractController.saveContractOverride = null
        }

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "contract-6")
        compare(contractController.lastSave.name, "New Contract")
        compare(contractController.lastSave.type, "modern")
        compare(contractController.lastSave.aliases.length, 2)
        compare(contractController.lastSave.aliases[0], "Base")
        compare(contractController.lastSave.aliases[1], "Alias Two")
        compare(contractController.lastSave.actorIds.length, 1)
        compare(contractController.lastSave.actorIds[0], "actor-1")
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        session.selectedContract = createContractObject({
            id: "contract-7",
            name: "Other",
            type: "legacy",
            actorIds: [],
            propertyIds: [],
            aliases: ["Other"]
        })
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedContract = createContractObject({
            id: "contract-6",
            name: "New Contract",
            type: "modern",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["Base", "Alias Two"]
        })
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Base")

        session.selectContractState("contract-6", "Old", "legacy", [], [], ["Base", "Alias Two"])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        session.selectContractState("contract-7", "Other", "legacy", [], [], ["Other"])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectContractState("contract-6", "Old", "legacy", [], [], ["Base", "Alias Two"])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
    }

    function test_dataRevisionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Base")

        session.selectedContract.aliases = ["Base", "Alias Two"]
        session.selectedContract.actorIds = ["actor-1"]
        session.selectedContract.propertyIds = ["property-1"]
        session.dataRevision += 1

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_actorSelectionUpdatesSelectedActorIds() {
        session.actors = [
            { id: "actor-1", name: "Alice" }
        ]
        var form = createForm(null)
        var actorCombo = findRequired(form, "contractActorComboBox")

        actorCombo.currentIndex = 1
        actorCombo.activated(1)

        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
    }

    function test_propertySelectionUpdatesSelectedPropertyIds() {
        session.properties = [
            { id: "property-1", name: "Lot" }
        ]
        var form = createForm(null)
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")

        propertyCheckBox.checked = true
        propertyCheckBox.toggled()

        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")

        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var createButton = findRequired(form, "contractCreateButton")
        nameField.text = "Lease 2026"
        typeField.text = "lease"
        typeField.textEdited()
        compare(createButton.enabled, true)
        createButton.clicked()
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
    }

    function test_createButtonStaysDisabledUntilAtLeastOneRelationIsSelected() {
        session.actors = [{ id: "actor-1", name: "Alice" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
        var form = createForm(null)
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var createButton = findRequired(form, "contractCreateButton")

        nameField.text = "Lease 2026"
        typeField.text = "lease"
        typeField.textEdited()

        compare(createButton.enabled, false)
        form.selectedActorIds = ["actor-1"]
        compare(createButton.enabled, true)
    }

    function test_navigationButtonsMoveSelectionId() {
        session.contracts = [
            { id: "contract-1", name: "A" },
            { id: "contract-2", name: "B" }
        ]
        var form = createForm({ id: "contract-1", name: "A", type: "x", actorIds: [], propertyIds: [], aliases: [] })
        var nextButton = findRequired(form, "contractNextButton")
        var previousButton = findRequired(form, "contractPreviousButton")

        nextButton.clicked()
        compare(session.selectedContractId, "contract-2")

        previousButton.clicked()
        compare(session.selectedContractId, "contract-1")
    }

    function test_createShortcutButtonClearsSelectionAndSwitchesToCreateMode() {
        var form = createForm({ id: "contract-9", name: "Selected", type: "base", actorIds: [], propertyIds: [], aliases: [] })
        var createModeButton = findRequired(form, "contractCreateModeButton")

        createModeButton.clicked()

        compare(session.selectedContractId, "")
        compare(form.isEdit, false)
    }

    function test_deleteButtonDeletesCurrentContract() {
        var form = createForm({ id: "contract-7", name: "Legacy", type: "base", actorIds: [], propertyIds: [], aliases: [] })
        var deleteButton = findRequired(form, "contractDeleteButton")

        deleteButton.clicked()

        compare(contractController.deleteCalls, 1)
        compare(contractController.lastDeleteId, "contract-7")
    }
}
