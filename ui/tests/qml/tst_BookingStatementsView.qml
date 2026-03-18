import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "BookingStatementsViewTests"
    when: windowShown
    width: 960
    height: 640

    property var uiData: QtObject {
        property var selectedStatement: null
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
    }

    property var statementController: QtObject {
        property int addCalls: 0
        property int updateCalls: 0
        property int deleteCalls: 0
        property var lastUpdate: ({})
        property string lastDeleteId: ""

        function reset() {
            addCalls = 0
            updateCalls = 0
            deleteCalls = 0
            lastUpdate = ({})
            lastDeleteId = ""
        }

        function addStatement(name) {
            addCalls += 1
            return "statement-new"
        }

        function updateStatement(id, name) {
            updateCalls += 1
            lastUpdate = { id: id, name: name }
        }

        function deleteStatement(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    Component {
        id: bookingStatementsComponent

        BookingStatementsView {
            width: 960
            height: 640
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createStatementObject(source) {
        if (!source)
            return null

        var statement = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: ""; property string name: "" }', testCase)
        statement.id = source.id || ""
        statement.name = source.name || ""
        return statement
    }

    function createView(statement) {
        var statementObject = createStatementObject(statement)
        uiData.selectedStatement = statementObject
        uiData.selectedStatementId = statementObject ? statementObject.id : ""
        uiData.selectedTransactionId = ""
        return createTemporaryObject(bookingStatementsComponent, testCase)
    }

    function init() {
        statementController.reset()
        uiData.selectedStatement = null
        uiData.selectedStatementId = ""
        uiData.selectedTransactionId = ""
    }

    function test_createModeAddsStatementAndStoresSelection() {
        var view = createView(null)
        var nameField = findRequired(view, "bookingStatementNameField")
        var submitButton = findRequired(view, "bookingStatementSubmitButton")

        nameField.text = "January Statement"
        submitButton.clicked()

        compare(statementController.addCalls, 1)
        compare(uiData.selectedStatementId, "statement-new")
        compare(nameField.text, "")
    }

    function test_editModeUpdatesStatement() {
        var view = createView({ id: "statement-1", name: "Old Name" })
        var nameField = findRequired(view, "bookingStatementNameField")
        var submitButton = findRequired(view, "bookingStatementSubmitButton")

        compare(nameField.text, "Old Name")
        nameField.text = "Updated Name"
        submitButton.clicked()

        compare(statementController.updateCalls, 1)
        compare(statementController.lastUpdate.id, "statement-1")
        compare(statementController.lastUpdate.name, "Updated Name")
    }

    function test_deleteAndNewTransactionUpdateUiSelectionState() {
        var view = createView({ id: "statement-2", name: "Existing" })
        var newTransactionButton = findRequired(view, "bookingStatementNewTransactionButton")
        var deleteButton = findRequired(view, "bookingStatementDeleteButton")

        newTransactionButton.clicked()
        compare(uiData.selectedTransactionId, "__new__")

        deleteButton.clicked()
        compare(statementController.deleteCalls, 1)
        compare(statementController.lastDeleteId, "statement-2")
        compare(uiData.selectedStatementId, "")
        compare(uiData.selectedTransactionId, "")
    }
}
