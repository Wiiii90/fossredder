/*!
 * @file ui/qml/FossRedder/Views/Actor/ActorForm.qml
 * @brief Main actor edit form with name, aliases, contract links, and bottom bar actions.
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
    readonly property var contractController: root.appContext ? root.appContext.contractController : null

    readonly property var current: root.session ? root.session.selectedActor : null
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    property var aliases: []
    property string aliasInputText: ""
    property int aliasIndex: aliases.length > 0 ? 0 : -1
    property var selectedContractIds: []

    function clearFields() {
        nameField.text = ""
        root.aliases = []
        root.aliasInputText = ""
        root.aliasIndex = -1
        root.selectedContractIds = []
    }

    function toStringList(values) {
        const out = []
        if (!values || values.length === undefined)
            return out
        for (let i = 0; i < values.length; ++i)
            out.push(String(values[i]))
        return out
    }

    function actorRows() {
        return root.session ? root.session.actorRows() : []
    }

    function contractRows() {
        return root.session ? root.session.contractRows() : []
    }

    function selectedActorIndex() {
        if (!root.session || !root.session.selectedActorId)
            return -1
        const rows = root.actorRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === root.session.selectedActorId)
                return i
        }
        return -1
    }

    function selectActorByIndex(index) {
        const rows = root.actorRows()
        if (!root.session || rows.length === 0)
            return
        let idx = index
        while (idx < 0)
            idx += rows.length
        idx = idx % rows.length
        const row = rows[idx]
        if (!row || !row.id)
            return
        root.session.selectedActorId = row.id
    }

    function navigateActor(delta) {
        const rows = root.actorRows()
        if (rows.length === 0)
            return
        if (!root.isEdit) {
            root.selectActorByIndex(delta > 0 ? 0 : rows.length - 1)
            return
        }
        const idx = root.selectedActorIndex()
        if (idx < 0)
            return
        root.selectActorByIndex(idx + delta)
    }

    function addAlias(value) {
        const trimmed = String(value || "").trim()
        if (trimmed.length === 0)
            return
        const next = root.aliases ? root.aliases.slice(0) : []
        if (next.indexOf(trimmed) !== -1)
            return
        next.push(trimmed)
        root.aliases = next
        root.aliasIndex = next.length - 1
        root.aliasInputText = ""
    }

    function deleteAlias(index) {
        if (!root.aliases || index < 0 || index >= root.aliases.length)
            return
        const next = root.aliases.slice(0)
        next.splice(index, 1)
        root.aliases = next
        root.aliasIndex = next.length > 0 ? Math.min(index, next.length - 1) : -1
    }


    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }
        nameField.text = root.current.name || ""
        root.aliases = root.current.aliases ? root.current.aliases.slice(0) : []
        root.aliasInputText = ""
        root.aliasIndex = root.aliases.length > 0 ? 0 : -1
        root.selectedContractIds = []
    }

    function submitActor() {
        if (!root.actorController) return
        const aliasValues = root.toStringList(root.aliases)

        if (root.isEdit) {
            root.actorController.updateActor(root.current.id, nameField.text, "", "", aliasValues)
            return
        }

        const id = root.actorController.addActor(nameField.text, "", "", aliasValues)
        root.clearFields()
        if (root.session && id && id.length > 0) root.session.selectedActorId = id
    }

    onCurrentChanged: root.syncFields()
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
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
                        Layout.fillWidth: true
                        placeholderText: ""
                        text: root.aliasInputText
                        onTextEdited: root.aliasInputText = text
                    }

                    Controls.Button {
                        text: "+"
                        Layout.preferredWidth: aliasControlsRow.aliasControlSize
                        Layout.preferredHeight: aliasControlsRow.aliasControlSize
                        bordered: true
                        fillColor: root.theme.surface
                        textColor: root.theme.textMuted
                        enabled: actorAliasInput.text.trim().length > 0
                        onClicked: root.addAlias(actorAliasInput.text)
                    }

                    Controls.Button {
                        text: "×"
                        Layout.preferredWidth: aliasControlsRow.aliasControlSize
                        Layout.preferredHeight: aliasControlsRow.aliasControlSize
                        bordered: true
                        fillColor: root.theme.surface
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

            Controls.Button {
                text: "◀"
                enabled: root.actorRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateActor(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
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
                onClicked: {
                    if (!root.actorController || !root.current || !root.current.id) return
                    root.actorController.deleteActor(root.current.id)
                    if (root.session) root.session.selectedActorId = ""
                }
            }

            Controls.SuccessButton {
                visible: root.isEdit
                text: qsTr("Update")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitActor()
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                text: "▶"
                enabled: root.actorRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateActor(1)
            }
        }
    }

    Component.onCompleted: {
        root.syncFields()
    }
}
