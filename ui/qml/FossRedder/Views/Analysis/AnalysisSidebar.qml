/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisSidebar.qml
 * @brief Provides the Analysis sidebar list.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var analysisState
    width: root.theme.shellSidebarPreferredWidth

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: analysisColumn.implicitHeight

            Column {
                id: analysisColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.analysisState.analysisRows

                    delegate: Rectangle {
                        id: analysisRow
                        objectName: "analysisSidebarRow"
                        required property var modelData
                        width: analysisColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: analysisRow.modelData.id === root.analysisState.selectedAnalysisId
                               ? root.theme.selectionHighlight
                               : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "analysisSidebarRowMouseArea"
                            anchors.fill: parent
                            onClicked: root.analysisState.selectAnalysis(analysisRow.modelData.id)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.name
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.type
                                color: root.theme.textMuted
                                elide: Text.ElideRight
                                visible: text.length > 0
                            }
                        }
                    }
                }
            }
        }
    }
}
