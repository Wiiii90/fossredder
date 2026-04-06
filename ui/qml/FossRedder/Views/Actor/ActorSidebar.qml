import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: actorColumn.implicitHeight

            Column {
                id: actorColumn
                width: parent.width
                spacing: Theme.spacingSmall

                Repeater {
                    model: session ? session.actorRows() : []

                    delegate: Rectangle {
                        width: actorColumn.width
                        height: 44
                        radius: 6
                        color: session && modelData.id === session.selectedActorId ? Theme.selectionHighlight : "transparent"
                        border.color: Theme.borderSoft
                        border.width: Theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (session) session.selectedActorId = modelData.id
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

