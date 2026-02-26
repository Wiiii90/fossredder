import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import components.controls 1.0
import "qrc:/qml/components/common"

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
        selectedActorIds = current.actorIds || []
        selectedPropertyIds = current.propertyIds || []
        try { if (actorPickerLoader && actorPickerLoader.item) actorPickerLoader.item.selectedIds = selectedActorIds } catch(e) {}
        try { if (propPickerLoader && propPickerLoader.item) propPickerLoader.item.selectedIds = selectedPropertyIds } catch(e) {}
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

        AppTextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        AppTextField { id: typeField; placeholderText: qsTr("Type"); Layout.fillWidth: true }
        AppTextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.preferredHeight: 120; wrapMode: TextArea.Wrap }

        GroupBox {
            title: qsTr("Actors (min 1)")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: Theme.spacingSmall

                Loader {
                    id: actorPickerLoader
                    Layout.fillWidth: true
                    source: "qrc:/qml/components/common/EntityPicker.qml"
                    onLoaded: {
                        if (!actorPickerLoader.item) return
                        actorPickerLoader.item.model = uiData ? uiData.actors : null
                        actorPickerLoader.item.selectedIds = selectedActorIds
                        actorPickerLoader.item.placeholder = qsTr("New actor name")
                        actorPickerLoader.item.showAdd = false
                        actorPickerLoader.item.added.connect(function(id) { if (selectedActorIds.indexOf(id) === -1) selectedActorIds.push(id) })
                        actorPickerLoader.item.selectedIdsChanged.connect(function() { selectedActorIds = actorPickerLoader.item.selectedIds })
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
                spacing: Theme.spacingSmall

                Loader {
                    id: propPickerLoader
                    Layout.fillWidth: true
                    source: "qrc:/qml/components/common/EntityPicker.qml"
                    onLoaded: {
                        if (!propPickerLoader.item) return
                        propPickerLoader.item.model = uiData ? uiData.properties : null
                        propPickerLoader.item.selectedIds = selectedPropertyIds
                        propPickerLoader.item.placeholder = qsTr("New property name")
                        propPickerLoader.item.showAdd = false
                        propPickerLoader.item.added.connect(function(id) { if (selectedPropertyIds.indexOf(id) === -1) selectedPropertyIds.push(id) })
                        propPickerLoader.item.selectedIdsChanged.connect(function() { selectedPropertyIds = propPickerLoader.item.selectedIds })
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing

            AppButton { visible: isEdit; text: qsTr("New"); onClicked: if (uiData) uiData.selectedContractId = "" }

            AppButton {
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

            AppButton { visible: isEdit; text: qsTr("Delete"); onClicked: { if (!uiDomain) return; uiDomain.deleteContract(current.id); if (uiData) uiData.selectedContractId = ""; clearFields() } }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: syncFields()
}
