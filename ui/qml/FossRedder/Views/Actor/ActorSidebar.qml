import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

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
            contentHeight: actorColumn.implicitHeight

            Column {
                id: actorColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.session ? root.session.actorRows() : []

                    delegate: Rectangle {
                        id: actorRow
                        required property var modelData
                        width: actorColumn.width
                        height: 44
                        radius: 6
                        color: root.session && actorRow.modelData.id === root.session.selectedActorId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session) root.session.selectedActorId = actorRow.modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: 2

                            Text {
                                id: actorNameText
                                width: parent.width
                                text: actorRow.modelData.name ? actorRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                id: actorTypeText
                                width: parent.width
                                text: actorRow.modelData.type ? actorRow.modelData.type : ""
                                color: root.theme.textMuted
                                elide: Text.ElideRight
                                visible: actorTypeText.text.length > 0
                            }
                        }
                    }
                }
            }
        }
    }
}

