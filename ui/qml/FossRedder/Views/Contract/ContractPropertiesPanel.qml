/*!
 * @file ui/qml/FossRedder/Views/Contract/ContractPropertiesPanel.qml
 * @brief Property selection panel used by the contract form with an internal scrollable list.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property var propertyRows: []
    property var selectedPropertyIds: []
    signal selectionChanged(var ids)

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
        anchors.fill: parent
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

                    delegate: RowLayout {
                        id: propertyRow
                        required property var modelData
                        readonly property string propertyId: propertyRow.modelData && propertyRow.modelData.id ? propertyRow.modelData.id : ""

                        width: propertyColumn.width
                        spacing: root.theme.spacingSmall

                        Controls.CheckBox {
                            Layout.fillWidth: false
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            checked: root.selectedPropertyIds.indexOf(propertyRow.propertyId) !== -1
                            onClicked: {
                                const next = root.selectedPropertyIds ? root.selectedPropertyIds.slice(0) : []
                                const idx = next.indexOf(propertyRow.propertyId)
                                if (checked && idx === -1)
                                    next.push(propertyRow.propertyId)
                                else if (!checked && idx !== -1)
                                    next.splice(idx, 1)
                                root.selectionChanged(next)
                            }
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
