/**
 * @file ui/qml/FossRedder/Views/Contract/ContractPropertiesPanel.qml
 * @brief Provides the ContractPropertiesPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var contractState
    property var propertyRows: []
    readonly property var selectedPropertyIds: root.contractState ? root.contractState.selectedPropertyIds : []

    Layout.fillWidth: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Properties")
            Layout.fillWidth: true
        }

        Flickable {
            id: propertyScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: propertyColumn.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            Column {
                id: propertyColumn
                width: propertyScroll.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.propertyRows

                    delegate: Item {
                        id: propertyRow
                        required property var modelData
                        readonly property string propertyId: propertyRow.modelData && propertyRow.modelData.id ? propertyRow.modelData.id : ""

                        width: propertyColumn.width
                        height: rowLayout.implicitHeight

                        RowLayout {
                            id: rowLayout
                            anchors.left: parent.left
                            anchors.right: parent.right
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "contractPropertyCheckBox"
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.selectedPropertyIds.indexOf(propertyRow.propertyId) !== -1
                                onToggled: if (root.contractState) root.contractState.setPropertySelected(propertyRow.propertyId, checked)
                            }

                            Label {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: propertyRow.modelData && propertyRow.modelData.name ? propertyRow.modelData.name : ""
                                elide: Text.ElideRight
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }
}
