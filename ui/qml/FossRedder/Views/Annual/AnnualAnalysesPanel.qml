/*!
 * @file ui/qml/FossRedder/Views/Annual/AnnualAnalysesPanel.qml
 * @brief Read-only panel that previews assigned analyses for the selected annual.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property var allAnalysisRows: []
    property var analysisRows: []
    property var selectedAnalysisIds: []

    signal selectionChanged(var ids)

    Layout.fillWidth: true
    Layout.fillHeight: true
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
            text: qsTr("Assigned analyses")
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            readonly property real actionButtonSize: Math.max(addAnalysisCombo.implicitHeight, root.theme.spacingLarge * 2)

            Controls.ComboBox {
                id: addAnalysisCombo
                Layout.fillWidth: true
                model: root.allAnalysisRows
                textRole: "display"
                currentIndex: -1
            }

            Controls.Button {
                text: "+"
                bordered: true
                Layout.preferredWidth: parent.actionButtonSize
                Layout.preferredHeight: parent.actionButtonSize
                fillColor: root.theme.surface
                textColor: root.theme.textMuted
                enabled: addAnalysisCombo.currentIndex >= 0
                onClicked: {
                    const row = addAnalysisCombo.currentIndex >= 0
                                ? addAnalysisCombo.model[addAnalysisCombo.currentIndex]
                                : null
                    const analysisId = row && row.id ? String(row.id) : ""
                    if (analysisId.length === 0)
                        return

                    const next = root.selectedAnalysisIds ? root.selectedAnalysisIds.slice() : []
                    if (next.indexOf(analysisId) !== -1)
                        return
                    next.push(analysisId)
                    root.selectionChanged(next)
                }
            }
        }

        Flickable {
            id: analysisScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: analysisList.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: analysisList
                width: analysisScroll.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.analysisRows

                    delegate: Rectangle {
                        id: analysisRow
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 34
                        radius: root.theme.radius
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.border

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.rightMargin: root.theme.spacingSmall
                            spacing: root.theme.spacingSmall

                            Label {
                                text: analysisRow.modelData && analysisRow.modelData.name
                                      ? analysisRow.modelData.name
                                      : ""
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: analysisRow.modelData && analysisRow.modelData.type
                                      ? analysisRow.modelData.type
                                      : ""
                                color: root.theme.textMuted
                                elide: Text.ElideRight
                            }

                            Controls.Button {
                                text: "×"
                                bordered: true
                                Layout.preferredWidth: root.theme.spacingLarge
                                Layout.preferredHeight: root.theme.spacingLarge
                                fillColor: root.theme.surface
                                textColor: root.theme.textMuted
                                onClicked: {
                                    const rowId = analysisRow.modelData && analysisRow.modelData.id
                                                  ? String(analysisRow.modelData.id)
                                                  : ""
                                    if (rowId.length === 0)
                                        return

                                    const next = root.selectedAnalysisIds ? root.selectedAnalysisIds.slice() : []
                                    const removeIndex = next.indexOf(rowId)
                                    if (removeIndex === -1)
                                        return
                                    next.splice(removeIndex, 1)
                                    root.selectionChanged(next)
                                }
                            }
                        }
                    }
                }

                Label {
                    visible: (root.analysisRows ? root.analysisRows.length : 0) === 0
                    text: qsTr("No analyses assigned")
                    color: root.theme.textMuted
                    Layout.fillWidth: true
                }
            }
        }
    }
}
