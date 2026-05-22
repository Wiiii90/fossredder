/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractPropertyPanel.qml
 * @brief Manages property selection and suggestion handling inside the transaction draft contract panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0
    property bool embedded: false
    property string newPropertyName: ""

    function normalizedText(value) {
        return String(value || "").trim().replace(/\s+/g, " ").toLowerCase()
    }

    function hasIdenticalProperty() {
        const name = normalizedText(root.newPropertyName)
        if (name.length === 0)
            return false
        const rows = root.propertyRows()
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (!row)
                continue
            const display = normalizedText(row.display || row.name || "")
            if (display === name)
                return true
        }
        return false
    }

    function addPropertyFromInput() {
        if (!root.txRoot || !root.txRoot.draft || !root.importWorkflow)
            return
        const name = String(root.newPropertyName || "").trim()
        if (name.length === 0)
            return
        root.importWorkflow.createPropertyChoiceForCurrentDraft(root.txRoot.draft, name)
        root.newPropertyName = ""
    }

    function propertyRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.propertyRows() : []
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.txRoot && root.txRoot.viewState
                    ? root.txRoot.suggestionColor({ confidence: Number(root.txRoot.viewState.propertySuggestionConfidence || 0) })
                    : root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.theme.spacingSmall

                Label { text: qsTr("Select Property"); Layout.fillWidth: true }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Repeater {
                        id: propertyRepeater
                        model: root.propertyRows()

                        delegate: RowLayout {
                            id: propertyOption
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.txRoot && root.txRoot.draft && root.txRoot.draft.current && root.txRoot.draft.current.propertyIds && propertyOption.modelData && propertyOption.modelData.id
                                    ? root.txRoot.draft.current.propertyIds.indexOf(propertyOption.modelData.id) !== -1
                                    : false
                                onToggled: if (root.txRoot && root.txRoot.draft && root.importWorkflow) {
                                    root.importWorkflow.setCurrentPropertySelected(root.txRoot.draft, propertyOption.modelData.id, checked)
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: propertyOption.modelData ? (propertyOption.modelData.display || propertyOption.modelData.name || "") : ""
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }

                    Label {
                        visible: propertyRepeater.count === 0
                        text: qsTr("No properties available")
                        color: root.theme.textMuted
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Label { text: qsTr("Name"); Layout.fillWidth: true }

                        Controls.TextField {
                            id: propertyNameField
                            objectName: "transactionDraftPropertyNameInput"
                            Layout.fillWidth: true
                            placeholderText: ""
                            text: root.newPropertyName
                            onTextEdited: root.newPropertyName = text
                            onAccepted: root.addPropertyFromInput()
                        }
                    }

                    ColumnLayout {
                        spacing: root.theme.spacingSmall
                        Label { text: " "; Layout.fillWidth: false }
                        Controls.SecondaryButton {
                            objectName: "transactionDraftPropertyAddButton"
                            text: qsTr("+")
                            Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                            Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                            Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                            Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                            Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                            Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                            textColor: root.theme.textMuted
                            enabled: String(root.newPropertyName || "").trim().length > 0 && !root.hasIdenticalProperty()
                            onClicked: root.addPropertyFromInput()
                        }
                    }
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.viewState && root.txRoot.viewState.propertySuggestionSummary
                ? String(root.txRoot.viewState.propertySuggestionSummary)
                : qsTr("0% Confidence - No suggestion")
            color: root.txRoot && root.txRoot.viewState
                ? root.txRoot.suggestionColor({ confidence: Number(root.txRoot.viewState.propertySuggestionConfidence || 0) })
                : root.theme.textMuted
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }

    Connections {
        target: root.txRoot ? root.txRoot.draft : null
        function onCurrentIndexChanged() { root.newPropertyName = "" }
        function onCurrentChanged() {}
        function onCountChanged() { root.newPropertyName = "" }
    }
}
