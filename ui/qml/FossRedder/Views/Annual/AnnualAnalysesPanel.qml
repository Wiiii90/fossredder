/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualAnalysesPanel.qml
 * @brief Provides the AnnualAnalysesPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var appContext
    required property var theme
    property var allAnalysisRows: []
    property var analysisRows: []
    property var selectedAnalysisIds: []
    readonly property var analysisController: root.appContext ? root.appContext.analysisController : null

    signal selectionChanged(var ids)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    function updateAnalysisExportFormat(row, exportFormat) {
        const analysis = row || ({})
        const analysisId = analysis.id ? String(analysis.id) : ""
        if (!root.analysisController || analysisId.length === 0)
            return

        root.analysisController.updateAnalysis(
            analysisId,
            analysis.name ? String(analysis.name) : "",
            analysis.type ? String(analysis.type) : "tab",
            analysis.config ? String(analysis.config) : "{}",
            analysis.filter ? String(analysis.filter) : "",
            exportFormat ? String(exportFormat).toLowerCase() : "",
            analysis.includeCalcAdjustments !== undefined ? !!analysis.includeCalcAdjustments : true,
            analysis.exportState ? String(analysis.exportState) : "{}",
            analysis.snapshotTransactions ? String(analysis.snapshotTransactions) : "{}")
        root.selectionChanged(root.selectedAnalysisIds ? root.selectedAnalysisIds.slice() : [])
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Rectangle {
                Layout.preferredWidth: 92
                Layout.preferredHeight: root.theme.controlHeight
                radius: root.theme.radius
                color: root.theme.surface
                border.width: root.theme.borderWidthThin
                border.color: root.theme.borderSoft

                Label {
                    anchors.fill: parent
                    text: qsTr("Analysis")
                    color: root.theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
            }

            Controls.DropdownMenu {
                id: addAnalysisCombo
                objectName: "annualAddAnalysisComboBox"
                Layout.fillWidth: true
                model: root.allAnalysisRows
                textRole: "display"
                currentIndex: model && model.length > 0 ? 0 : -1
            }

            Controls.SecondaryButton {
                objectName: "annualAddAnalysisButton"
                text: "+"
                Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                Layout.maximumHeight: root.theme.viewCompactActionButtonSize
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

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 160
            radius: root.theme.radius
            color: root.theme.surfaceAlt
            border.width: 1
            border.color: root.theme.border

            Flickable {
                id: analysisScroll
                anchors.fill: parent
                anchors.margins: root.theme.spacing
                clip: true
                contentWidth: width
                contentHeight: Math.max(analysisList.implicitHeight, analysisScroll.height)

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                Column {
                    id: analysisList
                    width: analysisScroll.width
                    spacing: root.theme.spacing

                    Repeater {
                        model: root.analysisRows

                        delegate: Rectangle {
                            id: analysisRow
                            required property var modelData
                            width: analysisList.width
                            implicitHeight: analysisContentRow.implicitHeight + (root.theme.spacing * 2)
                            height: implicitHeight
                            radius: root.theme.radius
                            color: "transparent"
                            border.width: root.theme.borderWidthThin
                            border.color: root.theme.borderSoft

                            RowLayout {
                                id: analysisContentRow
                                anchors.fill: parent
                                anchors.margins: root.theme.spacing
                                spacing: root.theme.spacing

                                Rectangle {
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        text: qsTr("Analysis")
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.bold: true
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        anchors.leftMargin: root.theme.spacing
                                        anchors.rightMargin: root.theme.spacing
                                        text: analysisRow.modelData && analysisRow.modelData.display
                                              ? analysisRow.modelData.display
                                              : ""
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }

                                Controls.DropdownMenu {
                                    objectName: "annualAnalysisExportFormatComboBox"
                                    Layout.preferredWidth: 110
                                    model: analysisRow.modelData && String(analysisRow.modelData.type).toLowerCase() === "plot"
                                           ? ["PNG", "JPG"]
                                           : ["XLSX", "CSV"]
                                    currentIndex: Math.max(0, model.indexOf(String(analysisRow.modelData && analysisRow.modelData.exportFormat ? analysisRow.modelData.exportFormat : "").toUpperCase()))
                                    onActivated: {
                                        const selectedExportFormat = model && currentIndex >= 0 ? model[currentIndex] : ""
                                        root.updateAnalysisExportFormat(analysisRow.modelData, selectedExportFormat)
                                    }
                                }

                                Controls.SecondaryButton {
                                    objectName: "annualRemoveAnalysisButton"
                                    text: "×"
                                    Layout.preferredWidth: root.theme.viewCompactActionButtonSize
                                    Layout.minimumWidth: root.theme.viewCompactActionButtonSize
                                    Layout.maximumWidth: root.theme.viewCompactActionButtonSize
                                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                    Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                    Layout.maximumHeight: root.theme.viewCompactActionButtonSize
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

                    Item {
                        width: parent.width
                        height: (root.analysisRows ? root.analysisRows.length : 0) === 0 ? Math.max(analysisScroll.height, emptyContent.implicitHeight) : 0
                        visible: (root.analysisRows ? root.analysisRows.length : 0) === 0

                        ColumnLayout {
                            id: emptyContent
                            anchors.fill: parent
                            anchors.margins: root.theme.spacing
                            spacing: root.theme.spacingSmall

                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }
}
