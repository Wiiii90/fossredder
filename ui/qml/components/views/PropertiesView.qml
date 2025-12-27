import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    property var current: uiData ? uiData.selectedProperty : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    function clearFields() {
        nameField.text = ""
        addressField.text = ""
        descField.text = ""
    }

    function syncFields() {
        if (!isEdit) {
            clearFields()
            return
        }
        nameField.text = current.name || ""
        addressField.text = current.address || ""
        descField.text = current.description || ""
    }

    Connections {
        target: current
        function onChanged() { syncFields() }
    }

    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label { text: isEdit ? qsTr("Edit Property") : qsTr("Create Property"); font.pointSize: 18 }

        TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        TextField { id: addressField; placeholderText: qsTr("Address"); Layout.fillWidth: true }
        TextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.fillHeight: true; wrapMode: TextArea.Wrap }

        RowLayout {
            Layout.fillWidth: true

            Button {
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (uiData) uiData.selectedPropertyId = ""
                }
            }

            Button {
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (isEdit) {
                        if (uiDomain) uiDomain.updateProperty(current.id, nameField.text, addressField.text, descField.text)
                    } else {
                        if (uiDomain) {
                            var id = uiDomain.addProperty(nameField.text, addressField.text, descField.text)
                            clearFields()
                            if (uiData && id && id.length > 0) uiData.selectedPropertyId = id
                        }
                    }
                }
            }

            Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!uiDomain) return
                    uiDomain.deleteProperty(current.id)
                    if (uiData) uiData.selectedPropertyId = ""
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
