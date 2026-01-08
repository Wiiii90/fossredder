import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"

Item {
    id: root

    property var current: uiData ? uiData.selectedContract : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    property var selectedActorIds: []
    property var selectedPropertyIds: []

    function clearFields() {
        nameField.text = ""
        typeField.text = ""
        descField.text = ""
        selectedActorIds = []
        selectedPropertyIds = []
        quickActorField.text = ""
        quickPropertyField.text = ""
    }

    function syncFields() {
        if (!isEdit) {
            clearFields()
            return
        }
        nameField.text = current.name || ""
        typeField.text = current.type || ""
        descField.text = current.description || ""
        selectedActorIds = current.actorIds || []
        selectedPropertyIds = current.propertyIds || []
    }

    function toggleId(arr, id, checked) {
        var out = arr.slice(0)
        var idx = out.indexOf(id)
        if (checked && idx < 0) out.push(id)
        if (!checked && idx >= 0) out.splice(idx, 1)
        return out
    }

    function canSubmit() {
        return nameField.text.length > 0 && selectedActorIds.length > 0 && selectedPropertyIds.length > 0
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

        Label { text: isEdit ? qsTr("Edit Contract") : qsTr("Create Contract"); font.pointSize: 18 }

        AppTextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        AppTextField { id: typeField; placeholderText: qsTr("Type"); Layout.fillWidth: true }
        TextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.preferredHeight: 120; wrapMode: TextArea.Wrap }

        GroupBox {
            title: qsTr("Actors (min 1)")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Repeater {
                    model: uiData ? uiData.actors : null
                    delegate: CheckBox {
                        text: model.name
                        checked: selectedActorIds.indexOf(model.id) >= 0
                        onToggled: {
                            selectedActorIds = toggleId(selectedActorIds, model.id, checked)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    AppTextField { id: quickActorField; placeholderText: qsTr("New actor name"); Layout.fillWidth: true }
                    Button {
                        text: qsTr("Add")
                        enabled: quickActorField.text.trim().length > 0
                        onClicked: {
                            if (!uiDomain) return
                            var id = uiDomain.ensureActorByName(quickActorField.text)
                            if (id && id.length > 0) {
                                selectedActorIds = toggleId(selectedActorIds, id, true)
                            }
                            quickActorField.text = ""
                        }
                    }
                }
            }
        }

        GroupBox {
            title: qsTr("Properties (min 1)")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Repeater {
                    model: uiData ? uiData.properties : null
                    delegate: CheckBox {
                        text: model.name
                        checked: selectedPropertyIds.indexOf(model.id) >= 0
                        onToggled: {
                            selectedPropertyIds = toggleId(selectedPropertyIds, model.id, checked)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    AppTextField { id: quickPropertyField; placeholderText: qsTr("New property name"); Layout.fillWidth: true }
                    Button {
                        text: qsTr("Add")
                        enabled: quickPropertyField.text.trim().length > 0
                        onClicked: {
                            if (!uiDomain) return
                            var id = uiDomain.ensurePropertyByName(quickPropertyField.text)
                            if (id && id.length > 0) {
                                selectedPropertyIds = toggleId(selectedPropertyIds, id, true)
                            }
                            quickPropertyField.text = ""
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (uiData) uiData.selectedContractId = ""
                }
            }

            Button {
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: canSubmit()
                onClicked: {
                    if (!uiDomain) return
                    if (isEdit) {
                        uiDomain.updateContract(current.id, nameField.text, typeField.text, descField.text, selectedActorIds, selectedPropertyIds)
                    } else {
                        var id = uiDomain.addContract(nameField.text, typeField.text, descField.text, selectedActorIds, selectedPropertyIds)
                        clearFields()
                        if (uiData && id && id.length > 0) uiData.selectedContractId = id
                    }
                }
            }

            Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!uiDomain) return
                    uiDomain.deleteContract(current.id)
                    if (uiData) uiData.selectedContractId = ""
                    clearFields()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}
