/**
 * @file ui/qml/FossRedder/Views/Actor/ActorContractPanel.qml
 * @brief Provides the ActorContractPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property var contractRows: []
    property var selectedContractIds: []
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
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Contracts")
            Layout.fillWidth: true
        }

        Flickable {
            id: contractScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractColumn.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            Column {
                id: contractColumn
                width: contractScroll.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.contractRows

                    delegate: RowLayout {
                        id: contractRow
                        required property var modelData
                        readonly property string contractId: contractRow.modelData && contractRow.modelData.id ? contractRow.modelData.id : ""

                        width: contractColumn.width
                        spacing: root.theme.spacingSmall

                        Controls.CheckBox {
                            objectName: "actorContractCheckBox"
                            Layout.fillWidth: false
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            checked: root.selectedContractIds.indexOf(contractRow.contractId) !== -1
                            onClicked: {
                                const next = root.selectedContractIds ? root.selectedContractIds.slice(0) : []
                                const idx = next.indexOf(contractRow.contractId)
                                if (checked && idx === -1)
                                    next.push(contractRow.contractId)
                                else if (!checked && idx !== -1)
                                    next.splice(idx, 1)
                                root.selectionChanged(next)
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            text: contractRow.modelData && contractRow.modelData.name ? contractRow.modelData.name : ""
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
