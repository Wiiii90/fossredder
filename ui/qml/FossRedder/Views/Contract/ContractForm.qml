/**
 * @file ui/qml/FossRedder/Views/Contract/ContractForm.qml
 * @brief Provides the ContractForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components
import FossRedder.Views 1.0 as Views
import FossRedder 1.0 as FossRedder
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var sessionState: root.appContext ? root.appContext.sessionState : null
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property var current: root.session ? root.session.selectedContract : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    property var selectedActorIds: []
    property var selectedPropertyIds: []
    property string contractSelectionOwnerId: ""
    property string savedContractSelectionOwnerId: ""
    property var aliases: []
    property string aliasInputText: ""
    property int aliasIndex: aliases.length > 0 ? 0 : -1
    property string contractType: ""
    property string savedName: ""
    property string savedContractType: ""
    property var savedSelectedActorIds: []
    property var savedSelectedPropertyIds: []
    property var savedAliases: []

    function applyFormState(state) {
        const next = state || ({})
        nameField.text = next.name || ""
        root.contractType = next.type || ""
        root.selectedActorIds = next.selectedActorIds || []
        root.selectedPropertyIds = next.selectedPropertyIds || []
        root.aliases = next.aliases || []
        root.aliasInputText = next.aliasInputText || ""
        root.aliasIndex = next.aliasIndex !== undefined ? next.aliasIndex : -1
    }

    function normalizedList(values) {
        const list = values ? values.slice() : []
        list.sort()
        return JSON.stringify(list)
    }

    function captureSavedState() {
        root.savedName = String(nameField.text || "")
        root.savedContractType = String(root.contractType || "")
        root.savedSelectedActorIds = root.selectedActorIds ? root.selectedActorIds.slice() : []
        root.savedSelectedPropertyIds = root.selectedPropertyIds ? root.selectedPropertyIds.slice() : []
        root.savedAliases = root.aliases ? root.aliases.slice() : []
        root.savedContractSelectionOwnerId = root.contractSelectionOwnerId
    }

    function hasChanges() {
        if (!root.isEdit)
            return root.canSubmit()
        return root.savedName !== String(nameField.text || "")
                || root.savedContractType !== String(root.contractType || "")
                || root.normalizedList(root.savedSelectedActorIds) !== root.normalizedList(root.selectedActorIds)
                || root.normalizedList(root.savedSelectedPropertyIds) !== root.normalizedList(root.selectedPropertyIds)
                || root.normalizedList(root.savedAliases) !== root.normalizedList(root.aliases)
    }

    function clearFields() {
        root.contractSelectionOwnerId = ""
        if (!root.session) {
            root.applyFormState({})
            return
        }
        root.applyFormState(root.sessionState.contractFormState("", "", [], [], []))
    }

    function toStringList(values) {
        const out = []
        const list = values || []
        for (var i = 0; i < list.length; ++i) {
            const item = list[i]
            if (item && typeof item === "object") {
                if (item.value !== undefined && String(item.value).length > 0)
                    out.push(String(item.value))
                else if (item.source !== undefined && String(item.source).length > 0)
                    out.push(String(item.source))
                else if (item.id !== undefined && String(item.id).length > 0)
                    out.push(String(item.id))
                else
                    out.push(String(item))
            } else {
                out.push(String(item))
            }
        }
        return out
    }

    function actorRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.actorRows() : []
    }

    function propertyRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.propertyRows() : []
    }

    function contractRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.contractRows() : []
    }

    function navigateContract(delta) {
        const rows = root.contractRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isEdit ? (root.session.selectedContractId || "") : ""
        const currentIndex = root.sessionState.indexOfId ? root.sessionState.indexOfId(rows, currentId) : -1
        if ((delta > 0 && currentIndex === rows.length - 1)
                || (delta < 0 && currentIndex === 0)) {
            root.session.selectedContractId = ""
            return
        }
        const nextIndex = currentIndex < 0
            ? (delta > 0 ? 0 : rows.length - 1)
            : currentIndex + delta
        const nextId = rows[nextIndex] && rows[nextIndex].id ? String(rows[nextIndex].id) : ""
        if (!nextId || nextId.length === 0)
            return
        root.session.selectedContractId = nextId
    }

    function addAlias(value) {
        if (!root.workspaceFacade)
            return
        const next = root.sessionState.addUniqueTrimmed(root.aliases || [], value || "")
        if (next.length === root.aliases.length)
            return
        root.aliases = next
        root.aliasIndex = next.length - 1
        root.aliasInputText = ""
    }

    function deleteAlias(index) {
        if (!root.workspaceFacade)
            return
        const next = root.sessionState.removeAt(root.aliases || [], index)
        if (next.length === root.aliases.length)
            return
        root.aliases = next
        root.aliasIndex = next.length > 0 ? Math.min(index, next.length - 1) : -1
    }

    function syncFields(forceReload) {
        if (!root.isEdit) {
            root.clearFields()
            root.captureSavedState()
            return
        }
        const currentId = root.current && root.current.id ? String(root.current.id) : ""
        const previousOwnerId = root.contractSelectionOwnerId
        if (!forceReload && previousOwnerId === currentId)
            return
        root.contractSelectionOwnerId = currentId
        const current = root.current
        const state = root.session
            ? root.sessionState.contractFormState(current && current.name ? current.name : "",
                                                  current && current.type ? current.type : "",
                                                  current && current.actorIds ? root.toStringList(current.actorIds) : [],
                                                  current && current.propertyIds ? root.toStringList(current.propertyIds) : [],
                                                  current && current.aliases ? root.toStringList(current.aliases) : [])
            : ({})
        root.applyFormState(state)
        root.captureSavedState()
    }

    function canSubmit() {
        return nameField.text.trim().length > 0
                && root.contractType.trim().length > 0
                && ((root.selectedActorIds && root.selectedActorIds.length > 0)
                    || (root.selectedPropertyIds && root.selectedPropertyIds.length > 0))
    }

    function submitContract() {
        if (!root.workspaceFacade)
            return
        const aliasValues = root.toStringList(root.aliases)
        const actorIds = root.selectedActorIds ? root.selectedActorIds.slice() : []
        const propertyIds = root.selectedPropertyIds ? root.selectedPropertyIds.slice() : []
        const currentId = root.isEdit && root.current && root.current.id ? root.current.id : ""
        const contractId = root.workspaceFacade.saveContract(currentId,
                                                                nameField.text,
                                                                root.contractType,
                                                                actorIds,
                                                                propertyIds,
                                                                aliasValues)
        if (root.session && contractId && contractId.length > 0)
            root.session.selectedContractId = contractId
        root.captureSavedState()
    }

    function deleteContract() {
        if (!root.workspaceFacade || !root.current || !root.current.id)
            return

        const removedId = root.current.id
        root.workspaceFacade.deleteContract(removedId)
        if (!root.session) {
            root.clearFields()
            return
        }

        const nextId = root.sessionState.deleteNextSelectionId(root.session.contractRows(), removedId, 0, "id")
        root.session.selectedContractId = nextId || ""
        if (!nextId || nextId.length === 0)
            root.clearFields()
    }

    Connections {
        target: root.session
        function onSelectedContractIdChanged() { root.syncFields() }
        function onDataRevisionChanged() { root.syncFields(true) }
    }
    Connections {
        target: root.current
        function onChanged() { root.syncFields(true) }
    }
    onCurrentChanged: root.syncFields(true)
    onIsEditChanged: root.syncFields(true)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.viewFormSpacing

        Flickable {
            id: contractScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
                contentHeight: contractContent.height
                boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contractContent
                width: contractScroll.width
                height: Math.max(implicitHeight, contractScroll.height)
                spacing: root.theme.viewFormSpacing

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Contract Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: nameField
                        objectName: "contractNameField"
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
                        readonly property real aliasControlSize: root.theme.viewCompactActionButtonSize

                        Label {
                            text: qsTr("Aliases")
                            Layout.preferredWidth: root.theme.formLabelWidth
                        }

                        Controls.TextField {
                            id: contractAliasInput
                            objectName: "contractAliasInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.aliasInputText
                            onTextChanged: root.aliasInputText = text
                            onTextEdited: root.aliasInputText = text
                        }

                        Controls.SecondaryButton {
                            objectName: "contractAddAliasButton"
                            text: qsTr("+")
                            Layout.preferredWidth: aliasControlsRow.aliasControlSize
                            Layout.minimumWidth: aliasControlsRow.aliasControlSize
                            Layout.maximumWidth: aliasControlsRow.aliasControlSize
                            Layout.preferredHeight: aliasControlsRow.aliasControlSize
                            Layout.minimumHeight: aliasControlsRow.aliasControlSize
                            Layout.maximumHeight: aliasControlsRow.aliasControlSize
                            textColor: root.theme.textMuted
                            enabled: contractAliasInput.text.trim().length > 0
                            onClicked: root.addAlias(contractAliasInput.text)
                        }

                        Controls.SecondaryButton {
                            objectName: "contractRemoveAliasButton"
                            text: qsTr("-")
                            Layout.preferredWidth: aliasControlsRow.aliasControlSize
                            Layout.minimumWidth: aliasControlsRow.aliasControlSize
                            Layout.maximumWidth: aliasControlsRow.aliasControlSize
                            Layout.preferredHeight: aliasControlsRow.aliasControlSize
                            Layout.minimumHeight: aliasControlsRow.aliasControlSize
                            Layout.maximumHeight: aliasControlsRow.aliasControlSize
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
                        id: contractAliasScroll
                        objectName: "contractAliasScroll"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: width
                        contentHeight: contractAliasFlow.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        Flow {
                            id: contractAliasFlow
                            width: contractAliasScroll.width
                            spacing: root.theme.spacingSmall

                            Repeater {
                                model: root.aliases

                                delegate: Rectangle {
                                    id: contractAliasChip
                                    required property var modelData
                                    required property int index
                                    height: root.theme.viewAliasChipHeight
                                    radius: root.theme.viewAliasChipRadius
                                    color: root.aliasIndex === contractAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.theme.border
                                    width: Math.min(contractAliasFlow.width, contractAliasText.implicitWidth + root.theme.spacingLarge)

                                    Text {
                                        id: contractAliasText
                                        anchors.centerIn: parent
                                        text: String(contractAliasChip.modelData)
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width - root.theme.spacing
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.aliasIndex = contractAliasChip.index
                                    }
                                }
                            }
                        }
                    }
                }
                }

                Views.ContractTypePanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    typeValue: root.contractType
                    onTypeEdited: (text) => root.contractType = text
                }

                Views.ContractActorsPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    Layout.maximumHeight: implicitHeight
                    theme: root.theme
                    sessionState: root.sessionState
                    actorRows: root.actorRows()
                    selectedActorIds: root.selectedActorIds
                    onSelectionChanged: function(ids) { root.selectedActorIds = ids }
                }

                Views.ContractPropertiesPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
                    theme: root.theme
                    propertyRows: root.propertyRows()
                    selectedPropertyIds: root.selectedPropertyIds
                    onSelectionChanged: function(ids) { root.selectedPropertyIds = ids }
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "contractPreviousButton"
                enabled: root.contractRows().length > 0
                onClicked: root.navigateContract(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "contractClearButton"
                visible: !root.isEdit
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFields()
            }

            Controls.SuccessButton {
                objectName: "contractCreateButton"
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: root.canSubmit()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitContract()
            }

            Controls.SecondaryButton {
                objectName: "contractCreateModeButton"
                visible: root.isEdit
                text: qsTr("+")
                Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                textColor: root.theme.textMuted
                onClicked: {
                    if (!root.session)
                        return
                    root.session.selectedContract = null
                    root.session.selectedContractId = ""
                    root.clearFields()
                }
            }

            Controls.DangerButton {
                objectName: "contractDeleteButton"
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteContract()
            }

            Controls.SuccessButton {
                objectName: "contractUpdateButton"
                visible: root.isEdit
                text: qsTr("Update")
                enabled: root.canSubmit() && root.hasChanges()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitContract()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "contractNextButton"
                enabled: root.contractRows().length > 0
                onClicked: root.navigateContract(1)
            }
        }
    }

    Component.onCompleted: root.syncFields()
}
