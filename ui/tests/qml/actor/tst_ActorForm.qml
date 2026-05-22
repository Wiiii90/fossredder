/**
 * @file ui/tests/qml/actor/tst_ActorForm.qml
 * @brief Provides QML tests for ActorForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedActor: null
        property string selectedActorId: ""
        property int dataRevision: 0
        property var actors: []
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

        function actorRows() {
            return actors || []
        }

        function contractRows() {
            return contracts || []
        }

        function indexOfId(rows, id) {
            var list = rows || []
            var target = String(id || "")
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === target)
                    return i
            }
            return -1
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

        function selectActorState(newId, newName, newAliases, newContractIds) {
            if (selectedActor && selectedActor.setState)
                selectedActor.setState(newId, newName, newAliases, newContractIds)
            selectedActorId = String(newId || "")
        }
    }

    property var actorController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var saveActorOverride: null

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            saveActorOverride = null
        }

        function defaultSaveActor(id, name, aliases, contractIds) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            }
            return id && id.length > 0 ? id : "actor-new"
        }

        function saveActor(id, name, aliases, contractIds) {
            if (saveActorOverride)
                return saveActorOverride(id, name, aliases, contractIds)
            return defaultSaveActor(id, name, aliases, contractIds)
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

        function deleteActor(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var sessionState: testCase.session
        property var workspaceFacade: testCase.actorController
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
        property int viewCompactActionButtonSize: 28
        property int viewAliasChipHeight: 24
        property int viewAliasChipRadius: 3
        property int borderWidthThin: 1
        property int radius: 3
    }

    Component {
        id: actorFormComponent

        ActorForm {
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

    function createActorObject(source) {
        if (!source)
            return null
        var actor = Qt.createQmlObject('import QtQml 2.15; QtObject { signal changed(); property string id: ""; property string name: ""; property var aliases: []; property var contractIds: []; function setState(newId, newName, newAliases, newContractIds) { id = newId || ""; name = newName || ""; aliases = newAliases || []; contractIds = newContractIds || []; changed(); } }', testCase)
        actor.setState(source.id || "", source.name || "", source.aliases || [], source.contractIds || [])
        return actor
    }

    function createForm(selectedActor) {
        var actorObject = createActorObject(selectedActor)
        session.selectedActor = actorObject
        session.selectedActorId = actorObject ? actorObject.id : ""
        return createTemporaryObject(actorFormComponent, testCase)
    }

    function init() {
        actorController.reset()
        session.selectedActor = null
        session.selectedActorId = ""
        session.actors = []
        session.contracts = []
    }

    function test_createModeSavesActorAndSelectsNewId() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var nameField = findRequired(form, "actorNameField")
        var submitButton = findRequired(form, "actorCreateButton")
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var checkBox = findRequired(form, "actorContractCheckBox")

        nameField.text = "Alice"
        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        checkBox.checked = true
        checkBox.toggled()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        actorController.saveActorOverride = function(id, name, aliases, contractIds) {
            var result = actorController.defaultSaveActor(id, name, aliases, contractIds)
            session.selectedActor = createActorObject({
                id: result,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            })
            session.selectedActorId = result
            session.dataRevision += 1
            return result
        }
        try {
            submitButton.clicked()
        } finally {
            actorController.saveActorOverride = null
        }

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "")
        compare(actorController.lastSave.name, "Alice")
        compare(actorController.lastSave.aliases.length, 1)
        compare(actorController.lastSave.aliases[0], "Alias One")
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
        compare(session.selectedActorId, "actor-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        session.selectedActor = createActorObject({ id: "actor-other", name: "Other", aliases: ["Other"], contractIds: [] })
        session.selectedActorId = "actor-other"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedActor = createActorObject({ id: "actor-new", name: "Alice", aliases: ["Alias One"], contractIds: ["contract-1"] })
        session.selectedActorId = "actor-new"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_aliasButtonsAddAndRemoveAlias() {
        var form = createForm(null)
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var removeAliasButton = findRequired(form, "actorRemoveAliasButton")

        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_editModeAliasButtonAddsAliasToFormState() {
        var form = createForm({ id: "actor-4", name: "Alice", aliases: ["Base"] })
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")

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
        var form = createForm({ id: "actor-4", name: "Alice", aliases: ["Base"] })
        var aliasScroll = findRequired(form, "actorAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
    }

    function test_readModeLoadsSelectedActorState() {
        var form = createForm({ id: "actor-3", name: "Bob", aliases: ["B", "B2"] })
        var nameField = findRequired(form, "actorNameField")

        compare(form.isEdit, true)
        compare(nameField.text, "Bob")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "B")
        compare(form.aliases[1], "B2")
    }

    function test_updateModeSavesCurrentActorId() {
        var form = createForm({ id: "actor-5", name: "Old", aliases: ["One"] })
        var nameField = findRequired(form, "actorNameField")
        var updateButton = findRequired(form, "actorUpdateButton")

        nameField.text = "New Name"
        updateButton.clicked()

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "actor-5")
        compare(actorController.lastSave.name, "New Name")
    }

    function test_updateModeKeepsSelectedContractsAndSavesThem() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })
        var nameField = findRequired(form, "actorNameField")
        var updateButton = findRequired(form, "actorUpdateButton")
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var checkBox = findRequired(form, "actorContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        nameField.text = "New Name"
        actorController.saveActorOverride = function(id, name, aliases, contractIds) {
            var result = actorController.defaultSaveActor(id, name, aliases, contractIds)
            if (session.selectedActor && session.selectedActor.setState)
                session.selectedActor.setState(id || "actor-6", name, aliases, contractIds)
            session.selectedActorId = id || "actor-6"
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            actorController.saveActorOverride = null
        }

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "actor-6")
        compare(actorController.lastSave.name, "New Name")
        compare(actorController.lastSave.aliases.length, 2)
        compare(actorController.lastSave.aliases[0], "One")
        compare(actorController.lastSave.aliases[1], "Alias Two")
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectedActor = createActorObject({ id: "actor-7", name: "Other", aliases: ["Other"], contractIds: [] })
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedActor = createActorObject({
            id: "actor-6",
            name: "New Name",
            aliases: ["One", "Alias Two"],
            contractIds: ["contract-1"]
        })
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectActorState("actor-6", "Old", ["One", "Alias Two"], [])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectActorState("actor-7", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectActorState("actor-6", "Old", ["One", "Alias Two"], [])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
    }

    function test_dataRevisionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectedActor.aliases = ["One", "Alias Two"]
        session.selectedActor.contractIds = ["contract-1"]
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
        var checkBox = findRequired(form, "actorContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        var updateButton = findRequired(form, "actorUpdateButton")
        var nameField = findRequired(form, "actorNameField")
        nameField.text = "Alice"
        updateButton.clicked()
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
    }

    function test_navigationButtonsMoveSelectionId() {
        session.actors = [
            { id: "actor-1", name: "A" },
            { id: "actor-2", name: "B" }
        ]
        var form = createForm({ id: "actor-1", name: "A", aliases: [] })
        var nextButton = findRequired(form, "actorNextButton")
        var previousButton = findRequired(form, "actorPreviousButton")

        nextButton.clicked()
        compare(session.selectedActorId, "actor-2")

        previousButton.clicked()
        compare(session.selectedActorId, "actor-1")
    }

    function test_createShortcutButtonClearsSelectionAndSwitchesToCreateMode() {
        var form = createForm({ id: "actor-9", name: "Selected", aliases: [] })
        var createModeButton = findRequired(form, "actorCreateModeButton")

        createModeButton.clicked()

        compare(session.selectedActorId, "")
        compare(form.isEdit, false)
    }

    function test_deleteButtonDeletesCurrentActor() {
        var form = createForm({ id: "actor-7", name: "Charlie", aliases: [] })
        var deleteButton = findRequired(form, "actorDeleteButton")

        deleteButton.clicked()

        compare(actorController.deleteCalls, 1)
        compare(actorController.lastDeleteId, "actor-7")
    }
}
