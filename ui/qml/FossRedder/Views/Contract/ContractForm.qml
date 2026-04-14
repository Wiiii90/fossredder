import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var contractController: root.appContext ? root.appContext.contractController : null

    readonly property var current: root.session ? root.session.selectedContract : null
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    property var selectedActorIds: []
    property var selectedPropertyIds: []
    property var aliases: []

    function clearFields() {
        nameField.text = ""
        typeField.text = ""
        descField.text = ""
        root.selectedActorIds = []
        root.selectedPropertyIds = []
        root.aliases = []
        aliasesField.text = ""
    }

    function parseAliases(text) {
        const aliasValues = []
        if (!text || text.length === 0) return aliasValues

        const raw = text.split(/\r?\n/)
        for (let i = 0; i < raw.length; ++i) {
            const value = String(raw[i]).trim()
            if (value.length > 0 && aliasValues.indexOf(value) === -1) aliasValues.push(value)
        }
        return aliasValues
    }

    function syncFields() {
        if (!root.isEdit) { root.clearFields(); return }
        nameField.text = root.current.name || ""
        typeField.text = root.current.type || ""
        descField.text = root.current.description || ""
        root.selectedActorIds = (root.current.actorIds || []).slice(0)
        root.selectedPropertyIds = (root.current.propertyIds || []).slice(0)
        root.aliases = (root.current.aliases || []).slice(0)
        aliasesField.text = root.aliases.join("\n")
        try { if (actorPicker) actorPicker.selectedIds = root.selectedActorIds.slice(0) } catch(e) {}
        try { if (propPicker) propPicker.selectedIds = root.selectedPropertyIds.slice(0) } catch(e) {}
    }

    function toggleId(arr, id, checked) {
        const out = arr.slice(0)
        const idx = out.indexOf(id)
        if (checked && idx < 0) out.push(id)
        if (!checked && idx >= 0) out.splice(idx, 1)
        return out
    }

    function canSubmit() { return nameField.text.length > 0 && root.selectedActorIds.length > 0 && root.selectedPropertyIds.length > 0 }

    Connections { target: root.current; function onChanged() { root.syncFields() } }
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageMargin
        spacing: root.theme.spacing

        Label { text: root.isEdit ? qsTr("Edit Contract") : qsTr("Create Contract"); font.pointSize: root.theme.fontSizeTitle + root.theme.margins }

        Controls.TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        Controls.TextField { id: typeField; placeholderText: qsTr("Type"); Layout.fillWidth: true }
        Controls.TextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.preferredHeight: root.theme.chartLegendHeight; wrapMode: TextArea.Wrap }

        GroupBox {
            title: qsTr("Actors (min 1)")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.theme.spacingMedium
                spacing: root.theme.spacingSmall

                Components.EntityPicker {
                    id: actorPicker
                    appContext: root.appContext
                    Layout.fillWidth: true
                    model: root.session ? root.session.actors : null
                    placeholder: qsTr("New actor name")
                    showAdd: false
                    onAdded: function(addedId) { if (root.selectedActorIds.indexOf(addedId) === -1) root.selectedActorIds.push(addedId) }
                    onSelectedIdsChanged: root.selectedActorIds = selectedIds ? selectedIds.slice(0) : []
                }
            }
        }

        GroupBox {
            title: qsTr("Properties (min 1)")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.theme.spacingMedium
                spacing: root.theme.spacingSmall

                Components.EntityPicker {
                    id: propPicker
                    appContext: root.appContext
                    Layout.fillWidth: true
                    model: root.session ? root.session.properties : null
                    placeholder: qsTr("New property name")
                    showAdd: false
                    onAdded: function(addedId) { if (root.selectedPropertyIds.indexOf(addedId) === -1) root.selectedPropertyIds.push(addedId) }
                    onSelectedIdsChanged: root.selectedPropertyIds = selectedIds ? selectedIds.slice(0) : []
                }
            }
        }

        GroupBox {
            title: qsTr("Aliases")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.theme.spacingMedium
                spacing: root.theme.spacingSmall

                Label {
                    text: qsTr("One alias per line. Used for auto-matching during import.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Controls.TextArea {
                    id: aliasesField
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.chartLegendHeight
                    wrapMode: TextArea.Wrap
                    placeholderText: qsTr("e.g.\nGasliefervertrag\nEON Vertrag\nHeizstrom")
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button { visible: root.isEdit; text: qsTr("New"); onClicked: if (root.session) root.session.selectedContractId = "" }

            Controls.Button {
                text: root.isEdit ? qsTr("Update") : qsTr("Add")
                enabled: root.canSubmit()
                onClicked: {
                    if (!root.contractController) return
                    const aliasValues = root.parseAliases(aliasesField.text)
                    if (root.isEdit) root.contractController.updateContract(root.current.id, nameField.text, typeField.text, descField.text, root.selectedActorIds, root.selectedPropertyIds, aliasValues)
                    else {
                        const contractId = root.contractController.addContract(nameField.text, typeField.text, descField.text, root.selectedActorIds, root.selectedPropertyIds, aliasValues)
                        root.clearFields()
                        if (root.session && contractId && contractId.length > 0) root.session.selectedContractId = contractId
                    }
                }
            }

            Controls.Button { visible: root.isEdit; text: qsTr("Delete"); onClicked: { if (!root.contractController) return; root.contractController.deleteContract(root.current.id); if (root.session) root.session.selectedContractId = ""; root.clearFields() } }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: root.syncFields()
}
