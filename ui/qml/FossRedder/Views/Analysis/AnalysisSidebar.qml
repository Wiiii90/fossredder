import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root
    width: 240

    Column {
        anchors.fill: parent
        spacing: 8

        Flickable {
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 40
            clip: true
            contentWidth: width
            contentHeight: analysisColumn.implicitHeight

            Column {
                id: analysisColumn
                width: parent.width
                spacing: Theme.spacingSmall

                Repeater {
                    model: session ? session.analysisRows() : []

                    delegate: Rectangle {
                        width: analysisColumn.width
                        height: 44
                        radius: 6
                        color: session && modelData.id === session.selectedAnalysisId ? Theme.selectionHighlight : "transparent"
                        border.color: Theme.borderSoft
                        border.width: Theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (session) session.selectedAnalysisId = modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: Theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: modelData.name ? modelData.name : ""
                                color: Theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: modelData.type ? modelData.type : ""
                                color: Theme.textMuted
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

