import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    property var current: uiData ? uiData.selectedActor : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    property var aliases: []

    function clearFields() {
        nameField.text = ""
        typeField.text = ""
        descField.text = ""
        aliases = []
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

        if (isEdit) {
            actorController.updateActor(current.id, nameField.text, typeField.text, descField.text)
            return
        }

        var id = actorController.addActor(nameField.text, typeField.text, descField.text)
        clearFields()
        if (uiData && id && id.length > 0) uiData.selectedActorId = id
    }

    Connections {
        target: current
        function onChanged() { syncFields() }
    }

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
            placeholderText: qsTr("Name")
            Layout.fillWidth: true
        }

        Controls.TextField {
            id: typeField
            placeholderText: qsTr("Type")
            Layout.fillWidth: true
        }

        Controls.TextArea {
            id: descField
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
                visible: isEdit
                text: qsTr("New")
                onClicked: {
                    if (uiData) uiData.selectedActorId = ""
                }
            }

            Controls.Button {
                text: isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: submitActor()
            }

            Controls.Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!actorController) return
                    actorController.deleteActor(current.id)
                    if (uiData) uiData.selectedActorId = ""
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        syncFields()
    }
}
