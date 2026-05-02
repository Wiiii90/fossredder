/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Analysis/AnalysisSidebar.qml
 * @brief Provides the AnalysisSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    width: 240
    readonly property var session: root.appContext ? root.appContext.session : null

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
                    model: root.session ? root.session.analyses : null

                    delegate: Rectangle { id: analysisRow
                        required property string id
                        required property string name
                        required property string type
                        width: analysisColumn.width
                        height: 44
                        radius: 6
                        color: root.session && analysisRow.id === root.session.selectedAnalysisId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session) root.session.selectedAnalysisId = analysisRow.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: analysisRow.name ? analysisRow.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: analysisRow.type ? analysisRow.type : ""
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

