import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    property var current: session ? session.selectedActor : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    property var aliases: []

    function clearFields() {
        nameField.text = ""
        typeField.text = ""
        descField.text = ""
        aliases = []
        aliasesField.text = ""
    }

    function syncFields() {
        if (!isEdit) {
            clearFields()
            return
        }
        nameField.text = current.name || ""
        typeField.text = current.type || ""
        descField.text = current.description || ""

        aliases = current.aliases ? current.aliases : []
        aliasesField.text = aliases.join("\n")
    }

    function submitActor() {
        if (!actorController) return

        var aliasValues = []
        if (aliasesField.text && aliasesField.text.length > 0) {
            var raw = aliasesField.text.split(/\r?\n/)
            for (var i = 0; i < raw.length; ++i) {
                var value = String(raw[i]).trim()
                if (value.length > 0 && aliasValues.indexOf(value) === -1) aliasValues.push(value)
            }
        }

        if (isEdit) {
            actorController.updateActor(current.id, nameField.text, typeField.text, descField.text, aliasValues)
            return
        }

        var id = actorController.addActor(nameField.text, typeField.text, descField.text, aliasValues)
        clearFields()
        if (session && id && id.length > 0) session.selectedActorId = id
    }

    onCurrentChanged: syncFields()
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.spacingMedium

        Label {
            text: isEdit ? qsTr("Edit Actor") : qsTr("Create Actor")
            font.pointSize: Theme.fontSizeTitle + Theme.margins
        }

        Controls.TextField {
            id: nameField
            objectName: "actorNameField"
            placeholderText: qsTr("Name")
            Layout.fillWidth: true
        }

        Controls.TextField {
            id: typeField
            objectName: "actorTypeField"
            placeholderText: qsTr("Type")
            Layout.fillWidth: true
        }

        Controls.TextArea {
            id: descField
            objectName: "actorDescriptionField"
            placeholderText: qsTr("Description")
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.chartLegendHeight
            wrapMode: TextArea.Wrap
        }

        GroupBox {
            title: qsTr("Aliases")
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingSmall

                Label {
                    text: qsTr("One alias per line. Used for auto-matching during import.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Controls.TextArea {
                    id: aliasesField
                    objectName: "actorAliasesField"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: TextArea.Wrap
                    placeholderText: qsTr("e.g.\nAMZN\nAmazon EU\nAmazon Payments")
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Controls.Button {
                objectName: "actorNewButton"
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (session) session.selectedActorId = ""
                }
            }

            Controls.Button {
                objectName: "actorSubmitButton"
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: submitActor()
            }

            Controls.Button {
                objectName: "actorDeleteButton"
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!actorController) return
                    actorController.deleteActor(current.id)
                    if (session) session.selectedActorId = ""
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        syncFields()
    }
}
