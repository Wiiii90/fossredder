/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisContractTypeFilter.qml
 * @brief Provides the AnalysisContractTypeFilter component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var analysisState
    readonly property var selectedTypes: root.analysisState.selectedContractTypes

    readonly property real actionButtonSize: root.theme.viewCompactActionButtonSize || root.theme.controlHeight || 32
    readonly property real actionButtonWidth: Math.max(root.actionButtonSize, 96)

    Layout.fillWidth: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: root.theme.borderWidthThin
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Contract Types")
                Layout.fillWidth: true
            }

            Controls.SecondaryButton {
                objectName: "analysisContractTypeFilterAllButton"
                text: qsTr("All")
                Layout.preferredWidth: root.actionButtonWidth
                Layout.preferredHeight: root.actionButtonSize
                onClicked: root.analysisState.selectAllContractTypes()
            }

            Controls.SecondaryButton {
                objectName: "analysisContractTypeFilterUnassignedButton"
                text: qsTr("Unassigned")
                Layout.preferredWidth: root.actionButtonWidth
                Layout.preferredHeight: root.actionButtonSize
                onClicked: root.analysisState.selectUnassignedContractTypes()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                radius: root.theme.radius
                color: root.theme.surface
                border.width: root.theme.borderWidthThin
                border.color: root.theme.border
            }

            Flickable {
                id: contractTypeScroll
                anchors.fill: parent
                anchors.margins: root.theme.panelPadding || root.theme.spacingSmall || 0
                clip: true
                contentWidth: width
                contentHeight: contractTypeColumn.implicitHeight

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                Column {
                    id: contractTypeColumn
                    width: contractTypeScroll.width
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: root.analysisState.contractTypeRows

                        delegate: RowLayout {
                            id: ctRow
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "analysisContractTypeFilterCheckBox"
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.selectedTypes.indexOf(String(ctRow.modelData.value)) !== -1
                                onClicked: root.analysisState.setContractTypeSelected(String(ctRow.modelData.value), checked)
                            }

                            Label {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: String(ctRow.modelData.label)
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item { Layout.fillWidth: true }
                        }
                    }
                }
            }
        }
    }
}
