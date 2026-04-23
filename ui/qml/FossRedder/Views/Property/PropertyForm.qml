/*!
 * @file ui/qml/FossRedder/Views/Property/PropertyForm.qml
 * @brief Main property edit form with name, aliases, contract links, and bottom bar actions.
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
    readonly property var propertyController: root.appContext ? root.appContext.propertyController : null
    readonly property var contractController: root.appContext ? root.appContext.contractController : null

    readonly property var current: root.session ? root.session.selectedProperty : null
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

    function propertyRows() {
        return root.session ? root.session.propertyRows() : []
    }

    function contractRows() {
        return root.session ? root.session.contractRows() : []
    }

    function selectedPropertyIndex() {
        if (!root.session || !root.session.selectedPropertyId)
            return -1
        const rows = root.propertyRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === root.session.selectedPropertyId)
                return i
        }
        return -1
    }

    function selectPropertyByIndex(index) {
        const rows = root.propertyRows()
        if (!root.session || rows.length === 0)
            return
        let idx = index
        while (idx < 0)
            idx += rows.length
        idx = idx % rows.length
        const row = rows[idx]
        if (!row || !row.id)
            return
        root.session.selectedPropertyId = row.id
    }

    function navigateProperty(delta) {
        const rows = root.propertyRows()
        if (rows.length === 0)
            return
        if (!root.isEdit) {
            root.selectPropertyByIndex(delta > 0 ? 0 : rows.length - 1)
            return
        }
        const idx = root.selectedPropertyIndex()
        if (idx < 0)
            return
        root.selectPropertyByIndex(idx + delta)
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


    function submitProperty() {
        if (!root.propertyController)
            return

        const aliasValues = root.toStringList(root.aliases)
        if (root.isEdit) {
            root.propertyController.updateProperty(root.current.id, nameField.text, "", "", aliasValues)
            return
        }

        const propertyId = root.propertyController.addProperty(nameField.text, "", "", aliasValues)
        root.clearFields()
        if (root.session && propertyId && propertyId.length > 0)
            root.session.selectedPropertyId = propertyId
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

    Connections {
        target: root.current
        function onChanged() { root.syncFields() }
    }
    onIsEditChanged: root.syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingMedium

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
                spacing: root.theme.spacingMedium

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Property Name")
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
                    readonly property real aliasControlSize: propertyAliasInput.implicitHeight

                    Label {
                        text: qsTr("Aliases")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        id: propertyAliasInput
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
                        enabled: propertyAliasInput.text.trim().length > 0
                        onClicked: root.addAlias(propertyAliasInput.text)
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
                        id: propertyAliasScroll
                        anchors.fill: parent
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
                                    property var aliasData: modelData
                                    property int aliasRowIndex: index
                                    height: 30
                                    radius: root.theme.radius
                                    color: root.aliasIndex === propertyAliasChip.aliasRowIndex ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                    border.width: 1
                                    border.color: root.theme.border
                                    width: Math.min(propertyAliasFlow.width, propertyAliasText.implicitWidth + root.theme.spacingLarge)

                                    Text {
                                        id: propertyAliasText
                                        anchors.centerIn: parent
                                        text: String(propertyAliasChip.aliasData)
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width - root.theme.spacing
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.aliasIndex = propertyAliasChip.aliasRowIndex
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
                Layout.minimumHeight: 170
                Layout.preferredHeight: 220
                theme: root.theme
                contractRows: root.contractRows()
                selectedContractIds: root.selectedContractIds
                onSelectionChanged: root.selectedContractIds = ids
            }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                text: "◀"
                enabled: root.propertyRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateProperty(-1)
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
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitProperty()
            }

            Controls.DangerButton {
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: {
                    if (!root.propertyController || !root.current || !root.current.id)
                        return
                    root.propertyController.deleteProperty(root.current.id)
                    if (root.session)
                        root.session.selectedPropertyId = ""
                }
            }

            Controls.SuccessButton {
                visible: root.isEdit
                text: qsTr("Update")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitProperty()
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                text: "▶"
                enabled: root.propertyRows().length > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateProperty(1)
            }
        }
    }

    Component.onCompleted: root.syncFields()
}
