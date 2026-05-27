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
    required property var actorState
    property var contractRows: []

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

                    delegate: Item {
                        id: contractRow
                        required property var modelData
                        readonly property string contractId: contractRow.modelData && contractRow.modelData.id ? contractRow.modelData.id : ""

                        width: contractColumn.width
                        height: rowLayout.implicitHeight

                        RowLayout {
                            id: rowLayout
                            anchors.left: parent.left
                            anchors.right: parent.right
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "actorContractCheckBox"
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.actorState ? root.actorState.isContractSelected(contractRow.contractId) : false
                                onToggled: {
                                    if (root.actorState)
                                        root.actorState.setContractSelected(contractRow.contractId, checked)
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
}
