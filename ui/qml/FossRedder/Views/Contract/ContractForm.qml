/*!
 * @file ui/qml/FossRedder/Views/Contract/ContractForm.qml
 * @brief Main contract edit form with name, aliases, type, actor/property links, and bottom bar actions.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components
import FossRedder.Views 1.0 as Views
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
    property string aliasInputText: ""
    property int aliasIndex: aliases.length > 0 ? 0 : -1
    property string contractType: ""

    function clearFields() {
        nameField.text = ""
        root.contractType = ""
        root.selectedActorIds = []
        root.selectedPropertyIds = []
        root.aliases = []
        root.aliasInputText = ""
        root.aliasIndex = -1
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

    function propertyRows() {
        return root.session ? root.session.propertyRows() : []
    }

    function contractRows() {
        return root.session ? root.session.contractRows() : []
    }

    function selectedContractIndex() {
        if (!root.session || !root.session.selectedContractId)
            return -1
        const rows = root.contractRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === root.session.selectedContractId)
                return i
        }
        return -1
    }

    function selectContractByIndex(index) {
        const rows = root.contractRows()
        if (!root.session || rows.length === 0)
            return
        let idx = index
        while (idx < 0)
            idx += rows.length
        idx = idx % rows.length
        const row = rows[idx]
        if (!row || !row.id)
            return
        root.session.selectedContractId = row.id
    }

    function navigateContract(delta) {
        const rows = root.contractRows()
        if (rows.length === 0)
            return
        if (!root.isEdit) {
            root.selectContractByIndex(delta > 0 ? 0 : rows.length - 1)
            return
        }
        const idx = root.selectedContractIndex()
        if (idx < 0)
            return
        root.selectContractByIndex(idx + delta)
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
        if (!root.isEdit) { root.clearFields(); return }
        nameField.text = root.current.name || ""
        root.contractType = root.current.type || ""
        const actorIds = root.toStringList(root.current.actorIds || [])
        root.selectedActorIds = actorIds.length > 0 ? [actorIds[0]] : []
        root.selectedPropertyIds = root.toStringList(root.current.propertyIds || [])
        root.aliases = (root.current.aliases || []).slice(0)
        root.aliasInputText = ""
        root.aliasIndex = root.aliases.length > 0 ? 0 : -1
    }

    function canSubmit() { return nameField.text.length > 0 }

    Connections { target: root.current; function onChanged() { root.syncFields() } }
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacing

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
                spacing: root.theme.spacing

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Contract Name")
                    Layout.preferredWidth: root.theme.formLabelWidth
                }

                Controls.TextField {
                    id: nameField
                    placeholderText: ""
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                RowLayout {
                    id: aliasControlsRow
                    Layout.fillWidth: true
                    readonly property real aliasControlSize: contractAliasInput.implicitHeight

                    Label {
                        text: qsTr("Aliases")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: contractAliasInput
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
                        enabled: contractAliasInput.text.trim().length > 0
                        onClicked: root.addAlias(contractAliasInput.text)
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
                    Layout.minimumHeight: 160
                    Layout.preferredHeight: 180
                    Layout.maximumHeight: 180
                    contentSpacing: 0
                    background: Rectangle {
                        radius: root.theme.radius
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.border
                    }

                    Flickable {
                        id: contractAliasScroll
                        anchors.fill: parent
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
                                    property var aliasData: modelData
                                    property int aliasRowIndex: index
                                    height: 30
                                    radius: root.theme.radius
                                    color: root.aliasIndex === contractAliasChip.aliasRowIndex ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.theme.border
                                    width: Math.min(contractAliasFlow.width, contractAliasText.implicitWidth + root.theme.spacingLarge)

                                    Text {
                                        id: contractAliasText
                                        anchors.centerIn: parent
                                        text: String(contractAliasChip.aliasData)
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width - root.theme.spacing
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.aliasIndex = contractAliasChip.aliasRowIndex
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
                onTypeEdited: root.contractType = text
            }

            Views.ContractActorsPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
                Layout.maximumHeight: implicitHeight
                theme: root.theme
                actorRows: root.actorRows()
                selectedActorIds: root.selectedActorIds
                onSelectionChanged: root.selectedActorIds = ids
            }

            Views.ContractPropertiesPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 170
                Layout.preferredHeight: 220
                theme: root.theme
                propertyRows: root.propertyRows()
                selectedPropertyIds: root.selectedPropertyIds
                onSelectionChanged: root.selectedPropertyIds = ids
            }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                text: "◀"
                enabled: root.contractRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateContract(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.DangerButton {
                visible: !root.isEdit
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFields()
            }

            Controls.SuccessButton {
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: root.canSubmit()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: {
                    if (!root.contractController) return
                    const aliasValues = root.toStringList(root.aliases)
                    const contractId = root.contractController.addContract(nameField.text, root.contractType, "", root.selectedActorIds, root.selectedPropertyIds, aliasValues)
                    root.clearFields()
                    if (root.session && contractId && contractId.length > 0) root.session.selectedContractId = contractId
                }
            }

            Controls.DangerButton {
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: {
                    if (!root.contractController || !root.current || !root.current.id) return
                    root.contractController.deleteContract(root.current.id)
                    if (root.session) root.session.selectedContractId = ""
                    root.clearFields()
                }
            }

            Controls.SuccessButton {
                visible: root.isEdit
                text: qsTr("Update")
                enabled: root.canSubmit()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: {
                    if (!root.contractController || !root.current || !root.current.id) return
                    const aliasValues = root.toStringList(root.aliases)
                    root.contractController.updateContract(root.current.id, nameField.text, root.contractType, "", root.selectedActorIds, root.selectedPropertyIds, aliasValues)
                }
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                text: "▶"
                enabled: root.contractRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateContract(1)
            }
        }
    }

    Component.onCompleted: root.syncFields()
}
