import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ActorFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedActor: null
        property string selectedActorId: ""
    }

    property var actorController: QtObject {
        property int addCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        property var lastAdd: ({})
        property var lastUpdate: ({})
        property string lastDeleteId: ""

        function reset() {
            addCalls = 0
            updateCalls = 0
            deleteCalls = 0
            lastAdd = ({})
            lastUpdate = ({})
            lastDeleteId = ""
        }

        function addActor(name, type, description) {
            addCalls += 1
            lastAdd = {
                name: name,
                type: type,
                description: description
            }
            return "actor-new"
        }

        function updateActor(id, name, type, description) {
            updateCalls += 1
            lastUpdate = {
                id: id,
                name: name,
                type: type,
                description: description
            }
        }

        function deleteActor(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    Component {
        id: actorFormComponent

        ActorForm {
            width: 960
            height: 640
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

        var actor = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: ""; property string name: ""; property string type: ""; property string description: "" }', testCase)
        actor.id = source.id || ""
        actor.name = source.name || ""
        actor.type = source.type || ""
        actor.description = source.description || ""
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
    }

    function test_createModeAddsActorAndStoresSelection() {
        var form = createForm(null)
        var nameField = findRequired(form, "actorNameField")
        var typeField = findRequired(form, "actorTypeField")
        var descriptionField = findRequired(form, "actorDescriptionField")
        var submitButton = findRequired(form, "actorSubmitButton")

        nameField.text = "Alice"
        typeField.text = "Vendor"
        descriptionField.text = "Preferred contractor"
        submitButton.clicked()

        compare(actorController.addCalls, 1)
        compare(actorController.lastAdd.name, "Alice")
        compare(actorController.lastAdd.type, "Vendor")
        compare(actorController.lastAdd.description, "Preferred contractor")
        compare(session.selectedActorId, "actor-new")
    }

    function test_editModeSyncsFieldsAndUpdatesActor() {
        var form = createForm({
            id: "actor-42",
            name: "Bob",
            type: "Owner",
            description: "Existing note"
        })
        var nameField = findRequired(form, "actorNameField")
        var typeField = findRequired(form, "actorTypeField")
        var descriptionField = findRequired(form, "actorDescriptionField")
        var submitButton = findRequired(form, "actorSubmitButton")

        compare(nameField.text, "Bob")
        compare(typeField.text, "Owner")
        compare(descriptionField.text, "Existing note")

        nameField.text = "Bobby"
        descriptionField.text = "Updated note"
        submitButton.clicked()

        compare(actorController.updateCalls, 1)
        compare(actorController.lastUpdate.id, "actor-42")
        compare(actorController.lastUpdate.name, "Bobby")
        compare(actorController.lastUpdate.type, "Owner")
        compare(actorController.lastUpdate.description, "Updated note")
    }

    function test_deleteButtonDeletesActorAndClearsSelection() {
        var form = createForm({
            id: "actor-7",
            name: "Charlie",
            type: "Tenant",
            description: "Removable"
        })
        var deleteButton = findRequired(form, "actorDeleteButton")

        deleteButton.clicked()

        compare(actorController.deleteCalls, 1)
        compare(actorController.lastDeleteId, "actor-7")
        compare(session.selectedActorId, "")
    }
}
