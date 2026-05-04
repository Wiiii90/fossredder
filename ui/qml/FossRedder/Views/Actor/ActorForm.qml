/**
 * @file ui/qml/FossRedder/Views/Actor/ActorForm.qml
 * @brief Provides the ActorForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
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
    property string aliasInputText: ""
    property int aliasIndex: aliases.length > 0 ? 0 : -1
    property var selectedContractIds: []
    property string savedName: ""
    property var savedAliases: []

    function applyFormState(state) {
        const next = state || ({})
        nameField.text = next.name || ""
        root.aliases = next.aliases || []
        root.aliasInputText = next.aliasInputText || ""
        root.aliasIndex = next.aliasIndex !== undefined ? next.aliasIndex : -1
        root.selectedContractIds = next.selectedIds || []
    }

    function normalizedList(values) {
        const list = values ? values.slice() : []
        list.sort()
        return JSON.stringify(list)
    }

    function captureSavedState() {
        root.savedName = String(nameField.text || "")
        root.savedAliases = root.aliases ? root.aliases.slice() : []
    }

    function hasChanges() {
        if (!root.isEdit)
            return nameField.text.length > 0
        return root.savedName !== String(nameField.text || "")
                || root.normalizedList(root.savedAliases) !== root.normalizedList(root.aliases)
    }

    function clearFields() {
        if (!root.session) {
            root.applyFormState({})
            return
        }
        root.applyFormState(root.session.basicFormState("", [], []))
    }

    function toStringList(values) {
        return root.session ? root.session.normalizeStrings(values || []) : []
    }

    function actorRows() {
        return root.session ? root.session.actorRows() : []
    }

    function contractRows() {
        return root.session ? root.session.contractRows() : []
    }

    function navigateActor(delta) {
        const rows = root.actorRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isEdit ? (root.session.selectedActorId || "") : ""
        const fallbackIndex = delta > 0 ? 0 : rows.length - 1
        const nextId = root.session.navigatedId(rows, currentId, delta, fallbackIndex)
        if (!nextId || nextId.length === 0)
            return
        root.session.selectedActorId = nextId
    }

    function addAlias(value) {
        if (!root.session)
            return
        const next = root.session.addUniqueTrimmed(root.aliases || [], value || "")
        if (next.length === root.aliases.length)
            return
        root.aliases = next
        root.aliasIndex = next.length - 1
        root.aliasInputText = ""
    }

    function deleteAlias(index) {
        if (!root.session)
            return
        const next = root.session.removeAt(root.aliases || [], index)
        if (next.length === root.aliases.length)
            return
        root.aliases = next
        root.aliasIndex = next.length > 0 ? Math.min(index, next.length - 1) : -1
    }


    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }
        const state = root.session
            ? root.session.basicFormState(root.current.name || "", root.current.aliases || [], [])
            : ({})
        root.applyFormState(state)
    }

    function submitActor() {
        if (!root.actorController) return
        const aliasValues = root.toStringList(root.aliases)
        const actorId = root.isEdit && root.current && root.current.id ? root.current.id : ""
        const id = root.actorController.saveActor(actorId, nameField.text, "", "", aliasValues)
        if (!root.isEdit)
            root.clearFields()
        if (root.session && id && id.length > 0)
            root.session.selectedActorId = id
        root.captureSavedState()
    }

    function deleteActor() {
        if (!root.actorController || !root.current || !root.current.id)
            return

        const removedId = root.current.id
        root.actorController.deleteActor(removedId)
        if (!root.session)
            return

        const nextId = root.session.deleteNextSelectionId(root.actorRows(), removedId, 0, "id")
        root.session.selectedActorId = nextId || ""
    }

    onCurrentChanged: root.syncFields()
    onIsEditChanged: root.syncFields()

    Connections {
        target: root.current
        function onChanged() { root.syncFields() }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.viewFormSpacing

        Flickable {
            id: actorScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: actorContent.height
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: actorContent
                width: actorScroll.width
                height: Math.max(implicitHeight, actorScroll.height)
                spacing: root.theme.viewFormSpacing

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Actor Name")
                    Layout.preferredWidth: root.theme.formLabelWidth
                }

                Controls.TextField {
                    id: nameField
                    objectName: "actorNameField"
                    placeholderText: ""
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.viewAliasGroupSpacing

                RowLayout {
                    id: aliasControlsRow
                    Layout.fillWidth: true
                    readonly property real aliasControlSize: actorAliasInput.implicitHeight

                    Label {
                        text: qsTr("Aliases")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: actorAliasInput
                        objectName: "actorAliasInput"
                        Layout.fillWidth: true
                        placeholderText: ""
                        text: root.aliasInputText
                        onTextEdited: root.aliasInputText = text
                    }

                    Controls.SecondaryButton {
                        objectName: "actorAddAliasButton"
                        text: qsTr("Add")
                        Layout.preferredWidth: aliasControlsRow.aliasControlSize
                        Layout.preferredHeight: aliasControlsRow.aliasControlSize
                        textColor: root.theme.textMuted
                        enabled: actorAliasInput.text.trim().length > 0
                        onClicked: root.addAlias(actorAliasInput.text)
                    }

                    Controls.SecondaryButton {
                        objectName: "actorRemoveAliasButton"
                        text: qsTr("Remove")
                        Layout.preferredWidth: aliasControlsRow.aliasControlSize
                        Layout.preferredHeight: aliasControlsRow.aliasControlSize
                        textColor: root.theme.textMuted
                        enabled: root.aliasIndex >= 0 && root.aliasIndex < root.aliases.length
                        onClicked: root.deleteAlias(root.aliasIndex)
                    }
                }

                Controls.Panel {
                    Layout.fillWidth: true
                    Layout.minimumHeight: root.theme.viewAliasPanelMinHeight
                    Layout.preferredHeight: root.theme.viewAliasPanelPreferredHeight
                    Layout.maximumHeight: root.theme.viewAliasPanelPreferredHeight
                    contentSpacing: 0
                    background: Rectangle {
                        radius: root.theme.radius
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.border
                    }

                    Flickable {
                        id: actorAliasScroll
                        anchors.fill: parent
                        clip: true
                        contentWidth: width
                        contentHeight: actorAliasFlow.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        Flow {
                            id: actorAliasFlow
                            width: actorAliasScroll.width
                            spacing: root.theme.spacingSmall

                            Repeater {
                                model: root.aliases

                                delegate: Rectangle {
                                    id: actorAliasChip
                                    required property var modelData
                                    required property int index
                                    height: 30
                                    radius: root.theme.radius
                                    color: root.aliasIndex === actorAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.theme.border
                                    width: Math.min(actorAliasFlow.width, actorAliasText.implicitWidth + root.theme.spacingLarge)

                                    Text {
                                        id: actorAliasText
                                        anchors.centerIn: parent
                                        text: String(actorAliasChip.modelData)
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width - root.theme.spacing
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.aliasIndex = actorAliasChip.index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Views.ActorContractPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
                theme: root.theme
                contractRows: root.contractRows()
                selectedContractIds: root.selectedContractIds
                onSelectionChanged: (ids) => root.selectedContractIds = ids
            }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "actorPreviousButton"
                enabled: root.actorRows().length > 0
                onClicked: root.navigateActor(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "actorClearButton"
                visible: !root.isEdit
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFields()
            }

            Controls.SuccessButton {
                objectName: "actorSubmitButton"
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitActor()
            }

            Controls.DangerButton {
                objectName: "actorDeleteButton"
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteActor()
            }

            Controls.SuccessButton {
                objectName: "actorUpdateButton"
                visible: root.isEdit
                text: qsTr("Update")
                enabled: root.hasChanges() && nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitActor()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "actorNextButton"
                enabled: root.actorRows().length > 0
                onClicked: root.navigateActor(1)
            }
        }
    }

    Component.onCompleted: {
        root.syncFields()
    }
}
