import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "EntityPickerTests"
    when: windowShown
    width: 720
    height: 480

    property var session: QtObject {
        property var actors: [
            { id: "actor-1", name: "Alice" }
        ]
        property var properties: [
            { id: "property-1", name: "Main Building" }
        ]
        property var contracts: [
            { id: "contract-1", name: "Lease" }
        ]
    }

    property var actorController: QtObject {
        property int addCalls: 0
        property string lastName: ""

        function reset() {
            addCalls = 0
            lastName = ""
        }

        function addActor(name, type, description) {
            addCalls += 1
            lastName = name
            return "actor-new"
        }
    }

    property var propertyController: QtObject {
        property int addCalls: 0

        function reset() {
            addCalls = 0
        }

        function addProperty(name, address, description) {
            addCalls += 1
            return "property-new"
        }
    }

    property var contractController: QtObject {
        property int addCalls: 0

        function reset() {
            addCalls = 0
        }

        function addContract(name, type, description, actorIds, propertyIds) {
            addCalls += 1
            return "contract-new"
        }
    }

    Component {
        id: actorPickerComponent

        EntityPicker {
            width: 720
            height: 480
            model: session.actors
        }
    }

    Component {
        id: contractPickerComponent

        EntityPicker {
            width: 720
            height: 480
            model: session.contracts
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createPicker(component, selectedIds) {
        return createTemporaryObject(component, testCase, {
            selectedIds: selectedIds || []
        })
    }

    function init() {
        actorController.reset()
        propertyController.reset()
        contractController.reset()
    }

    function test_addActorRoutesThroughActorControllerAndUpdatesSelection() {
        var picker = createPicker(actorPickerComponent, [])
        var addField = findRequired(picker, "entityPickerAddField")
        var addButton = findRequired(picker, "entityPickerAddButton")

        addField.text = "  New Actor  "
        addButton.clicked()

        compare(actorController.addCalls, 1)
        compare(actorController.lastName, "New Actor")
        compare(picker.selectedIds.length, 1)
        compare(picker.selectedIds[0], "actor-new")
        compare(addField.text, "")
    }

    function test_addContractRoutesThroughContractControllerAndAvoidsDuplicateSelection() {
        var picker = createPicker(contractPickerComponent, ["contract-new"])
        var addField = findRequired(picker, "entityPickerAddField")
        var addButton = findRequired(picker, "entityPickerAddButton")

        addField.text = "Lease Extension"
        addButton.clicked()

        compare(contractController.addCalls, 1)
        compare(picker.selectedIds.length, 1)
        compare(picker.selectedIds[0], "contract-new")
    }
}
