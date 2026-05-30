/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualSidebar.qml
 * @brief Provides the Annual sidebar list.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var annualState
    required property var theme
    width: root.theme.shellSidebarPreferredWidth

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: annualColumn.implicitHeight

            Column {
                id: annualColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.annualState.annualRows

                    delegate: Rectangle {
                        id: annualRow
                        objectName: "annualSidebarRow"
                        required property var modelData
                        width: annualColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: annualRow.modelData.id === root.annualState.selectedAnnualId
                               ? root.theme.selectionHighlight
                               : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "annualSidebarRowMouseArea"
                            anchors.fill: parent
                            onClicked: root.annualState.selectAnnual(annualRow.modelData.id)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                width: parent.width
                                text: annualRow.modelData.display
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: String(annualRow.modelData.year)
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
