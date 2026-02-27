import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

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
    }

    function syncFields() {
        if (!isEdit) { clearFields(); return }
        nameField.text = current.name || ""
        typeField.text = current.type || ""
        descField.text = current.description || ""
        selectedActorIds = (current.actorIds || []).slice(0)
        selectedPropertyIds = (current.propertyIds || []).slice(0)
        try { if (actorPicker) actorPicker.selectedIds = selectedActorIds.slice(0) } catch(e) {}
        try { if (propPicker) propPicker.selectedIds = selectedPropertyIds.slice(0) } catch(e) {}
    }

    function toggleId(arr, id, checked) {
        var out = arr.slice(0)
        var idx = out.indexOf(id)
        if (checked && idx < 0) out.push(id)
        if (!checked && idx >= 0) out.splice(idx, 1)
        return out
    }

    function canSubmit() { return nameField.text.length > 0 && selectedActorIds.length > 0 && selectedPropertyIds.length > 0 }

    Connections { target: current; function onChanged() { syncFields() } }
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: Theme.spacing

        Label { text: isEdit ? qsTr("Edit Contract") : qsTr("Create Contract"); font.pointSize: 18 }

        Controls.TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        Controls.TextField { id: typeField; placeholderText: qsTr("Type"); Layout.fillWidth: true }
        Controls.TextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.preferredHeight: 120; wrapMode: TextArea.Wrap }

        GroupBox {
            title: qsTr("Actors (min 1)")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: Theme.spacingSmall

                Components.EntityPicker {
                    id: actorPicker
                    Layout.fillWidth: true
                    model: uiData ? uiData.actors : null
                    placeholder: qsTr("New actor name")
                    showAdd: false
                    onAdded: { if (selectedActorIds.indexOf(id) === -1) selectedActorIds.push(id) }
                    onSelectedIdsChanged: selectedActorIds = selectedIds ? selectedIds.slice(0) : []
                }
            }
        }

        GroupBox {
            title: qsTr("Properties (min 1)")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: Theme.spacingSmall

                Components.EntityPicker {
                    id: propPicker
                    Layout.fillWidth: true
                    model: uiData ? uiData.properties : null
                    placeholder: qsTr("New property name")
                    showAdd: false
                    onAdded: { if (selectedPropertyIds.indexOf(id) === -1) selectedPropertyIds.push(id) }
                    onSelectedIdsChanged: selectedPropertyIds = selectedIds ? selectedIds.slice(0) : []
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing

            Controls.Button { visible: isEdit; text: qsTr("New"); onClicked: if (uiData) uiData.selectedContractId = "" }

            Controls.Button {
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: canSubmit()
                onClicked: {
                    if (!uiDomain) return
                    if (isEdit) uiDomain.updateContract(current.id, nameField.text, typeField.text, descField.text, selectedActorIds, selectedPropertyIds)
                    else {
                        var id = uiDomain.addContract(nameField.text, typeField.text, descField.text, selectedActorIds, selectedPropertyIds)
                        clearFields()
                        if (uiData && id && id.length > 0) uiData.selectedContractId = id
                    }
                }
            }

            Controls.Button { visible: isEdit; text: qsTr("Delete"); onClicked: { if (!uiDomain) return; uiDomain.deleteContract(current.id); if (uiData) uiData.selectedContractId = ""; clearFields() } }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: syncFields()
}
