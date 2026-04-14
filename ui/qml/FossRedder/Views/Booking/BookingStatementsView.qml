import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var statementController: root.appContext ? root.appContext.statementController : null

    property var current: root.session ? root.session.selectedStatement : null
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    function clearFields() {
        nameField.text = ""
    }

    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }
        nameField.text = root.current.name || ""
    }

    onCurrentChanged: root.syncFields()
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: root.isEdit ? qsTr("Edit Statement") : qsTr("Create Statement")
            font.pointSize: 18
        }

        Controls.TextField { id: nameField; objectName: "bookingStatementNameField"; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        Item { Layout.fillHeight: true }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                objectName: "bookingStatementNewButton"
                visible: root.isEdit
                text: qsTr("New")
                onClicked: {
                    if (root.session) {
                        root.session.selectedStatementId = ""
                        root.session.selectedTransactionId = ""
                    }
                }
            }

            Controls.Button {
                objectName: "bookingStatementSubmitButton"
                text: root.isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (!root.statementController) return
                    if (root.isEdit) {
                        root.statementController.updateStatement(root.current.id, nameField.text)
                    } else {
                        const id = root.statementController.addStatement(nameField.text)
                        root.clearFields()
                        if (root.session && id && id.length > 0) root.session.selectedStatementId = id
                    }
                }
            }

            Controls.Button {
                objectName: "bookingStatementDeleteButton"
                visible: root.isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!root.statementController) return
                    root.statementController.deleteStatement(root.current.id)
                    if (root.session) {
                        root.session.selectedStatementId = ""
                        root.session.selectedTransactionId = ""
                    }
                    root.clearFields()
                }
            }

            Controls.Button {
                objectName: "bookingStatementNewTransactionButton"
                visible: root.isEdit
                text: qsTr("New Transaction")
                onClicked: {
                    if (!root.session) return
                    root.session.selectedTransactionId = "__new__"
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        root.syncFields()
    }
}
