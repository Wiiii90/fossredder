import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    property var current: uiData ? uiData.selectedStatement : null
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

    Connections { target: current; function onChanged() { syncFields() } }
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: isEdit ? qsTr("Edit Statement") : qsTr("Create Statement")
            font.pointSize: 18
        }

        TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true

            Button {
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (uiData) {
                        uiData.selectedStatementId = ""
                        uiData.selectedTransactionId = ""
                    }
                }
            }

            Button {
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (!uiDomain) return
                    if (isEdit) {
                        uiDomain.updateStatement(current.id, nameField.text)
                    } else {
                        var id = uiDomain.addStatement(nameField.text)
                        clearFields()
                        if (uiData && id && id.length > 0) uiData.selectedStatementId = id
                    }
                }
            }

            Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!uiDomain) return
                    uiDomain.deleteStatement(current.id)
                    if (uiData) {
                        uiData.selectedStatementId = ""
                        uiData.selectedTransactionId = ""
                    }
                    clearFields()
                }
            }

            Button {
                visible: isEdit
                text: qsTr("New Transaction")
                onClicked: {
                    if (!uiData) return
                    uiData.selectedTransactionId = "__new__"
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
