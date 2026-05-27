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
    property var analysisMapper: null
    property var allAnalysisRows: []
    property var analysisRows: []
    property var selectedAnalysisIds: []
    readonly property var analysisWorkflow: root.appContext ? root.appContext.analysisWorkflow : null
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null

    signal selectionChanged(var ids)
    signal analysisOptionsChanged()

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    function updateAnalysisExportFormat(row, exportFormat) {
        const analysis = row || ({})
        const analysisId = analysis.id ? String(analysis.id) : ""
        if (analysisId.length === 0)
            return

        const nextFormat = exportFormat ? String(exportFormat).toLowerCase() : ""
        const payloadName = analysis.name ? String(analysis.name) : ""
        const payloadType = analysis.type ? String(analysis.type) : "tab"
        const payloadConfig = analysis.config ? String(analysis.config) : "{}"
        const payloadFilter = analysis.filter ? String(analysis.filter) : ""
        const payloadIncludeCalc = analysis.includeCalcAdjustments !== undefined ? !!analysis.includeCalcAdjustments : true
        const payloadExportState = analysis.exportState ? String(analysis.exportState) : "{}"
        const payloadSnapshotTransactions = analysis.snapshotTransactions ? String(analysis.snapshotTransactions) : "[]"

        if (!root.workspaceFacade || !root.workspaceFacade.updateAnalysis)
            return
        root.workspaceFacade.updateAnalysis(
            analysisId,
            payloadName,
            payloadType,
            payloadConfig,
            payloadFilter,
            nextFormat,
            payloadIncludeCalc,
            payloadExportState,
            payloadSnapshotTransactions)
        root.analysisOptionsChanged()
    }

    function analysisIdForRow(row) {
        if (root.analysisMapper && root.analysisMapper.analysisIdFromRow)
            return String(root.analysisMapper.analysisIdFromRow(row))
        return row && row.id ? String(row.id) : ""
    }

    function copyIds(values) {
        const out = []
        if (!values || values.length === undefined)
            return out
        for (let i = 0; i < values.length; ++i)
            out.push(String(values[i]))
        return out
    }

    function analysisTypeLabel(row) {
        const typeText = row && row.type ? String(row.type).toLowerCase() : "tab"
        if (typeText === "plot")
            return qsTr("Plot")
        if (typeText === "tab")
            return qsTr("Table")
        return qsTr("Table")
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
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

            Controls.AddButton {
                objectName: "annualAddAnalysisButton"
                Layout.preferredWidth: 72
                Layout.minimumWidth: 72
                Layout.maximumWidth: 72
                Layout.preferredHeight: root.theme.controlHeight
                Layout.minimumHeight: root.theme.controlHeight
                Layout.maximumHeight: root.theme.controlHeight
                textColor: root.theme.textMuted
                enabled: addAnalysisCombo.currentIndex >= 0
                onClicked: {
                    const row = addAnalysisCombo.currentIndex >= 0
                                ? addAnalysisCombo.model[addAnalysisCombo.currentIndex]
                                : null
                    const analysisId = root.analysisIdForRow(row)
                    if (root.appContext && root.appContext.isDebugBuild)
                        console.log("[AnnualAnalysesPanel] add click", "index=", addAnalysisCombo.currentIndex, "row=", JSON.stringify(row), "id=", analysisId)
                    if (analysisId.length === 0)
                        return

                    const next = root.copyIds(root.selectedAnalysisIds)
                    if (next.indexOf(analysisId) !== -1)
                        return
                    next.push(analysisId)
                    if (root.appContext && root.appContext.isDebugBuild)
                        console.log("[AnnualAnalysesPanel] emit selectionChanged", JSON.stringify(next))
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
                anchors.margins: root.theme.spacingSmall
                clip: true
                contentWidth: width
                contentHeight: Math.max(analysisList.implicitHeight, analysisScroll.height)

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                Column {
                    id: analysisList
                    width: analysisScroll.width
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: root.analysisRows || []

                        delegate: Rectangle {
                            id: analysisRow
                            required property var modelData
                            property var rowData: modelData
                            width: analysisList.width
                            implicitHeight: analysisContentRow.implicitHeight + (root.theme.spacingSmall * 2)
                            height: implicitHeight
                            radius: root.theme.radius
                            color: root.theme.surface
                            border.width: 1
                            border.color: root.theme.border

                            RowLayout {
                                id: analysisContentRow
                                anchors.fill: parent
                                anchors.leftMargin: root.theme.spacingSmall
                                anchors.rightMargin: root.theme.spacingSmall
                                anchors.topMargin: root.theme.spacingSmall
                                anchors.bottomMargin: root.theme.spacingSmall
                                spacing: root.theme.spacingSmall

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 220
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        anchors.leftMargin: root.theme.spacingSmall
                                        anchors.rightMargin: root.theme.spacingSmall
                                        text: {
                                            if (analysisRow.rowData && analysisRow.rowData.name)
                                                return String(analysisRow.rowData.name)
                                            if (analysisRow.rowData && analysisRow.rowData.display)
                                                return String(analysisRow.rowData.display)
                                            if (analysisRow.rowData && analysisRow.rowData.id)
                                                return String(analysisRow.rowData.id)
                                            return ""
                                        }
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignLeft
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: root.theme.controlHeight
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: root.theme.borderSoft

                                    Label {
                                        anchors.fill: parent
                                        text: root.analysisTypeLabel(analysisRow.rowData)
                                        color: root.theme.textPrimary
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.bold: true
                                    }
                                }

                                Controls.DropdownMenu {
                                    objectName: "annualAnalysisExportFormatComboBox"
                                    Layout.preferredWidth: 110
                                    model: analysisRow.rowData && String(analysisRow.rowData.type).toLowerCase() === "plot"
                                           ? ["PNG", "JPG"]
                                           : ["XLSX", "CSV"]
                                    currentIndex: Math.max(0, model.indexOf(String(analysisRow.rowData && analysisRow.rowData.exportFormat ? analysisRow.rowData.exportFormat : "").toUpperCase()))
                                    onActivated: {
                                        const selectedExportFormat = model && currentIndex >= 0 ? model[currentIndex] : ""
                                        root.updateAnalysisExportFormat(analysisRow.rowData, selectedExportFormat)
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
                                        const rowId = analysisRow.rowData && analysisRow.rowData.id
                                                      ? String(analysisRow.rowData.id)
                                                      : ""
                                        if (rowId.length === 0)
                                            return

                                        const next = root.copyIds(root.selectedAnalysisIds)
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
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: root.theme.spacingSmall

                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }
}
