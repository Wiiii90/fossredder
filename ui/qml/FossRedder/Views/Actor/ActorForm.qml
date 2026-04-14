import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var actorController: root.appContext ? root.appContext.actorController : null

    readonly property var current: root.session ? root.session.selectedActor : null
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    property var aliases: []

    function clearFields() {
        nameField.text = ""
        typeField.text = ""
        descField.text = ""
        root.aliases = []
        aliasesField.text = ""
    }

    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }
        nameField.text = root.current.name || ""
        typeField.text = root.current.type || ""
        descField.text = root.current.description || ""

        root.aliases = root.current.aliases ? root.current.aliases : []
        aliasesField.text = root.aliases.join("\n")
    }

    function submitActor() {
        if (!root.actorController) return

        const aliasValues = []
        if (aliasesField.text && aliasesField.text.length > 0) {
            const raw = aliasesField.text.split(/\r?\n/)
            for (let i = 0; i < raw.length; ++i) {
                const value = String(raw[i]).trim()
                if (value.length > 0 && aliasValues.indexOf(value) === -1) aliasValues.push(value)
            }
        }

        if (root.isEdit) {
            root.actorController.updateActor(root.current.id, nameField.text, typeField.text, descField.text, aliasValues)
            return
        }

        const id = root.actorController.addActor(nameField.text, typeField.text, descField.text, aliasValues)
        root.clearFields()
        if (root.session && id && id.length > 0) root.session.selectedActorId = id
    }

    onCurrentChanged: root.syncFields()
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageMargin
        spacing: root.theme.spacingMedium

        Label {
            text: root.isEdit ? qsTr("Edit Actor") : qsTr("Create Actor")
            font.pointSize: root.theme.fontSizeTitle + root.theme.margins
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
            Layout.preferredHeight: root.theme.chartLegendHeight
            wrapMode: TextArea.Wrap
        }

        GroupBox {
            title: qsTr("Aliases")
            Layout.fillWidth: true
            Layout.fillHeight: true

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
                    objectName: "actorAliasesField"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: TextArea.Wrap
                    placeholderText: qsTr("e.g.\nAMZN\nAmazon EU\nAmazon Payments")
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                objectName: "actorNewButton"
                visible: root.isEdit
                text: qsTr("New")
                onClicked: {
                    if (root.session) root.session.selectedActorId = ""
                }
            }

            Controls.Button {
                objectName: "actorSubmitButton"
                text: root.isEdit ? qsTr("Update") : qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: root.submitActor()
            }

            Controls.Button {
                objectName: "actorDeleteButton"
                visible: root.isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!root.actorController) return
                    root.actorController.deleteActor(root.current.id)
                    if (root.session) root.session.selectedActorId = ""
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        root.syncFields()
    }
}
