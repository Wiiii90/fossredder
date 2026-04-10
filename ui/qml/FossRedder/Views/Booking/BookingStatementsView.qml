import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    readonly property StateFacade session: AppContext.session
    readonly property StatementController statementController: AppContext.statementController

    property var current: session ? session.selectedStatement : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    function clearFields() {
        nameField.text = ""
    }

    function syncFields() {
        if (!isEdit) {
            clearFields()
            return
        }
        nameField.text = current.name || ""
    }

    onCurrentChanged: syncFields()
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: isEdit ? qsTr("Edit Statement") : qsTr("Create Statement")
            font.pointSize: 18
        }

        Controls.TextField { id: nameField; objectName: "bookingStatementNameField"; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true

            Controls.Button {
                objectName: "bookingStatementNewButton"
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (session) {
                        session.selectedStatementId = ""
                        session.selectedTransactionId = ""
                    }
                }
            }

            Controls.Button {
                objectName: "bookingStatementSubmitButton"
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (!statementController) return
                    if (isEdit) {
                        statementController.updateStatement(current.id, nameField.text)
                    } else {
                        var id = statementController.addStatement(nameField.text)
                        clearFields()
                        if (session && id && id.length > 0) session.selectedStatementId = id
                    }
                }
            }

            Controls.Button {
                objectName: "bookingStatementDeleteButton"
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!statementController) return
                    statementController.deleteStatement(current.id)
                    if (session) {
                        session.selectedStatementId = ""
                        session.selectedTransactionId = ""
                    }
                    clearFields()
                }
            }

            Controls.Button {
                objectName: "bookingStatementNewTransactionButton"
                visible: isEdit
                text: qsTr("New Transaction")
                onClicked: {
                    if (!session) return
                    session.selectedTransactionId = "__new__"
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        syncFields()
    }
}
