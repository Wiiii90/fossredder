import QtQuick 2.15
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    width: 240
    readonly property var session: root.appContext ? root.appContext.session : null

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
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.session ? root.session.analysisRows() : []

                    delegate: Rectangle { id: analysisRow
                        required property var modelData
                        width: analysisColumn.width
                        height: 44
                        radius: 6
                        color: root.session && analysisRow.modelData.id === root.session.selectedAnalysisId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session) root.session.selectedAnalysisId = analysisRow.modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.name ? analysisRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: analysisRow.modelData.type ? analysisRow.modelData.type : ""
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

