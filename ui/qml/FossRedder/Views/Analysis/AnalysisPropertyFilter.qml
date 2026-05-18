/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisPropertyFilter.qml
 * @brief Provides the AnalysisPropertyFilter component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var propertyRows
    property var selectedIds: []
    signal selectionChanged(var ids)
    readonly property real actionButtonSize: root.theme.viewCompactActionButtonSize || root.theme.controlHeight || 32
    readonly property real actionButtonWidth: Math.max(root.actionButtonSize, 56)

    function allIds() {
        const ids = []
        const rows = root.propertyRows || []
        for (let i = 0; i < rows.length; ++i) {
            const id = rows[i] && rows[i].id ? String(rows[i].id) : ""
            if (id.length > 0)
                ids.push(id)
        }
        return ids
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
                text: qsTr("Properties")
                Layout.fillWidth: true
            }

            Controls.SecondaryButton {
                objectName: "analysisPropertyFilterAllButton"
                text: qsTr("All")
                Layout.preferredWidth: root.actionButtonWidth
                Layout.preferredHeight: root.actionButtonSize
                onClicked: root.selectionChanged(root.allIds())
            }

            Controls.SecondaryButton {
                objectName: "analysisPropertyFilterNoneButton"
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
                id: propertyScroll
                anchors.fill: parent
                anchors.margins: root.theme.panelPadding || root.theme.spacingSmall || 0
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
                            id: rowRoot
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: root.theme.spacingSmall

                            Controls.CheckBox {
                                objectName: "analysisPropertyFilterCheckBox"
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                checked: root.selectedIds.indexOf(rowRoot.modelData.id) !== -1
                                onClicked: {
                                    const next = root.selectedIds ? root.selectedIds.slice() : []
                                    const idx = next.indexOf(rowRoot.modelData.id)
                                    if (checked && idx === -1)
                                        next.push(rowRoot.modelData.id)
                                    if (!checked && idx !== -1)
                                        next.splice(idx, 1)
                                    root.selectionChanged(next)
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: rowRoot.modelData && rowRoot.modelData.name ? rowRoot.modelData.name : ""
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
