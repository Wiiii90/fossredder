/**
 * @file ui/qml/FossRedder/Views/Property/PropertyForm.qml
 * @brief Handles property create/update/delete flows and related form state.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
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
    readonly property var current: root.session ? root.session.selectedProperty : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0

    property var aliases: []
    property string aliasInputText: ""
    property int aliasIndex: aliases.length > 0 ? 0 : -1
    property var selectedContractIds: []
    property string contractSelectionOwnerId: ""
    property var savedSelectedContractIds: []
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
        root.savedSelectedContractIds = root.selectedContractIds ? root.selectedContractIds.slice() : []
    }

    function hasChanges() {
        if (!root.isEdit)
            return nameField.text.length > 0
        return root.savedName !== String(nameField.text || "")
                || root.normalizedList(root.savedAliases) !== root.normalizedList(root.aliases)
                || root.normalizedList(root.savedSelectedContractIds) !== root.normalizedList(root.selectedContractIds)
    }

    function clearFields() {
        root.contractSelectionOwnerId = ""
        if (!root.session) {
            root.applyFormState({})
            return
        }
        root.applyFormState(root.sessionState.basicFormState("", [], []))
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

    function propertyRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.propertyRows() : []
    }

    function contractRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.contractRows() : []
    }

    function navigateProperty(delta) {
        const rows = root.propertyRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isEdit ? (root.session.selectedPropertyId || "") : ""
        const currentIndex = root.sessionState.indexOfId ? root.sessionState.indexOfId(rows, currentId) : -1
        if ((delta > 0 && currentIndex === rows.length - 1)
                || (delta < 0 && currentIndex === 0)) {
            root.session.selectedPropertyId = ""
            return
        }
        const nextIndex = currentIndex < 0
            ? (delta > 0 ? 0 : rows.length - 1)
            : currentIndex + delta
        const nextId = rows[nextIndex] && rows[nextIndex].id ? String(rows[nextIndex].id) : ""
        if (!nextId || nextId.length === 0)
            return
        root.session.selectedPropertyId = nextId
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


    function submitProperty() {
        if (!root.workspaceFacade)
            return
        const aliasValues = root.toStringList(root.aliases)
        const contractIds = root.selectedContractIds ? root.selectedContractIds.slice() : []
        const currentId = root.isEdit && root.current && root.current.id ? root.current.id : ""
        const propertyId = root.workspaceFacade.saveProperty(currentId, nameField.text, aliasValues, contractIds)
        if (root.session && propertyId && propertyId.length > 0)
            root.session.selectedPropertyId = propertyId
        root.captureSavedState()
    }

    function deleteProperty() {
        if (!root.workspaceFacade || !root.current || !root.current.id)
            return

        const removedId = root.current.id
        root.workspaceFacade.deleteProperty(removedId)
        if (!root.session)
            return

        const nextId = root.sessionState.deleteNextSelectionId(root.session.propertyRows(), removedId, 0, "id")
        root.session.selectedPropertyId = nextId || ""
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
            ? root.sessionState.basicFormState(current && current.name ? current.name : "",
                                               current && current.aliases ? root.toStringList(current.aliases) : [],
                                               current && current.contractIds ? root.toStringList(current.contractIds) : [])
            : ({})
        root.applyFormState(state)
        root.captureSavedState()
    }

    Connections {
        target: root.session
        function onSelectedPropertyIdChanged() { root.syncFields() }
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
            id: propertyScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: propertyContent.height
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: propertyContent
                width: propertyScroll.width
                height: Math.max(implicitHeight, propertyScroll.height)
                spacing: root.theme.viewFormSpacing

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Property Name")
                    Layout.preferredWidth: root.theme.formLabelWidth
                }

                Controls.TextField {
                    id: nameField
                    objectName: "propertyNameField"
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
                        id: propertyAliasInput
                        objectName: "propertyAliasInput"
                        Layout.fillWidth: true
                        placeholderText: ""
                        text: root.aliasInputText
                        onTextChanged: root.aliasInputText = text
                        onTextEdited: root.aliasInputText = text
                    }

                    Controls.SecondaryButton {
                        objectName: "propertyAddAliasButton"
                        text: qsTr("+")
                        Layout.preferredWidth: aliasControlsRow.aliasControlSize
                        Layout.minimumWidth: aliasControlsRow.aliasControlSize
                        Layout.maximumWidth: aliasControlsRow.aliasControlSize
                        Layout.preferredHeight: aliasControlsRow.aliasControlSize
                        Layout.minimumHeight: aliasControlsRow.aliasControlSize
                        Layout.maximumHeight: aliasControlsRow.aliasControlSize
                        textColor: root.theme.textMuted
                        enabled: propertyAliasInput.text.trim().length > 0
                        onClicked: root.addAlias(propertyAliasInput.text)
                    }

                    Controls.SecondaryButton {
                        objectName: "propertyRemoveAliasButton"
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
                        id: propertyAliasScroll
                        objectName: "propertyAliasScroll"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: width
                        contentHeight: propertyAliasFlow.implicitHeight

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        Flow {
                            id: propertyAliasFlow
                            width: propertyAliasScroll.width
                            spacing: root.theme.spacingSmall

                            Repeater {
                                model: root.aliases

                                delegate: Rectangle {
                                    id: propertyAliasChip
                                    required property var modelData
                                    required property int index
                                    height: root.theme.viewAliasChipHeight
                                    radius: root.theme.viewAliasChipRadius
                                    color: root.aliasIndex === propertyAliasChip.index ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.theme.border
                                    width: Math.min(propertyAliasFlow.width, propertyAliasText.implicitWidth + root.theme.spacingLarge)

                                    Text {
                                        id: propertyAliasText
                                        anchors.centerIn: parent
                                        text: String(propertyAliasChip.modelData)
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width - root.theme.spacing
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.aliasIndex = propertyAliasChip.index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Views.PropertyContractPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
                theme: root.theme
                contractRows: root.contractRows()
                selectedContractIds: root.selectedContractIds
                onSelectionChanged: function(ids) { root.selectedContractIds = ids }
            }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "propertyPreviousButton"
                enabled: root.propertyRows().length > 0
                onClicked: root.navigateProperty(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                objectName: "propertyClearButton"
                visible: !root.isEdit
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFields()
            }

            Controls.SuccessButton {
                objectName: "propertyCreateButton"
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitProperty()
            }

                Controls.SecondaryButton {
                    objectName: "propertyCreateModeButton"
                    visible: root.isEdit
                    text: qsTr("+")
                    Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                    Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                    Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                    textColor: root.theme.textMuted
                    onClicked: {
                        if (!root.session)
                            return
                    root.session.selectedProperty = null
                        root.session.selectedPropertyId = ""
                        root.clearFields()
                    }
                }

                Controls.DangerButton {
                    objectName: "propertyDeleteButton"
                    visible: root.isEdit
                    text: qsTr("Delete")
                    Layout.preferredWidth: root.theme.viewActionButtonWidth
                    onClicked: root.deleteProperty()
                }

            Controls.SuccessButton {
                objectName: "propertyUpdateButton"
                visible: root.isEdit
                text: qsTr("Update")
                enabled: nameField.text.length > 0 && root.hasChanges()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitProperty()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "propertyNextButton"
                enabled: root.propertyRows().length > 0
                onClicked: root.navigateProperty(1)
            }
        }
    }

    Component.onCompleted: root.syncFields()
}
