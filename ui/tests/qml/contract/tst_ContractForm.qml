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
    }

    property var contractController: QtObject {
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

        function saveContract(id, name, type, actorIds, propertyIds, aliases) {
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

        function deleteContract(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var contractController: testCase.contractController
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
        var contractObject = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: ""; property string name: ""; property string type: ""; property var actorIds: []; property var propertyIds: []; property var aliases: [] }', testCase)
        contractObject.id = source.id || ""
        contractObject.name = source.name || ""
        contractObject.type = source.type || ""
        contractObject.actorIds = source.actorIds || []
        contractObject.propertyIds = source.propertyIds || []
        contractObject.aliases = source.aliases || []
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
        var form = createForm(null)
        var nameField = findRequired(form, "contractNameField")
        var createButton = findRequired(form, "contractCreateButton")

        nameField.text = "Lease 2026"
        createButton.clicked()

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "")
        compare(contractController.lastSave.name, "Lease 2026")
        compare(session.selectedContractId, "contract-new")
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
        propertyCheckBox.clicked()

        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
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
