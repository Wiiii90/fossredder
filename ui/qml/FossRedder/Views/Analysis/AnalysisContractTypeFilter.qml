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
    required property var contractTypes
    property var selectedTypes: []
    signal selectionChanged(var selected)
    readonly property real actionButtonSize: root.theme.viewCompactActionButtonSize || root.theme.controlHeight || 32
    readonly property real actionButtonWidth: Math.max(root.actionButtonSize, 56)

    function allTypes() {
        const out = []
        const rows = root.contractTypes || []
        for (let i = 0; i < rows.length; ++i) {
            const value = String(rows[i] || "")
            if (value.length > 0)
                out.push(value)
        }
        return out
    }

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
                onClicked: root.selectionChanged(root.allTypes())
            }

            Controls.SecondaryButton {
                objectName: "analysisContractTypeFilterNoneButton"
                text: qsTr("None")
                Layout.preferredWidth: root.actionButtonWidth
                Layout.preferredHeight: root.actionButtonSize
                onClicked: root.selectionChanged([])
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Flickable {
                id: contractTypeScroll
                anchors.fill: parent
                anchors.margins: root.theme.panelPadding || root.theme.spacingSmall || 0
                clip: true
                contentWidth: width
                contentHeight: contractTypeColumn.implicitHeight

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                Column {
                    id: contractTypeColumn
                    width: contractTypeScroll.width
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: root.contractTypes
                        delegate: RowLayout {
                            id: ctRow
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "analysisContractTypeFilterCheckBox"
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.selectedTypes.indexOf(String(ctRow.modelData)) !== -1
                                onClicked: {
                                    const value = String(ctRow.modelData)
                                    const next = root.selectedTypes ? root.selectedTypes.slice() : []
                                    const idx = next.indexOf(value)
                                    if (checked && idx === -1)
                                        next.push(value)
                                    if (!checked && idx !== -1)
                                        next.splice(idx, 1)
                                    root.selectionChanged(next)
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: String(ctRow.modelData)
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
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
