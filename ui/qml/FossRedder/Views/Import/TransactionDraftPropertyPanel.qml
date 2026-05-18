/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftPropertyPanel.qml
 * @brief Manages property selection and suggestion handling for the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var importWorkflow: root.appContext ? root.appContext.importWorkflow : null
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0

    function propertyRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.propertyRows() : []
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label { text: qsTr("Property"); Layout.fillWidth: true }

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

        Label {
            text: root.txRoot ? root.txRoot.propertySuggestionSummary() : qsTr("No property suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }

    }
}
