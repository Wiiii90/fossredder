/**
 * @file ui/qml/FossRedder/Views/Actor/ActorSidebar.qml
 * @brief Provides the ActorSidebar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var actorRows: root.session ? root.session.actorRows : []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            objectName: "actorSidebarFlick"
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
                    model: root.actorRows

                    delegate: Rectangle {
                        id: actorRow
                        objectName: "actorSidebarRow_" + actorRow.actorId
                        required property var modelData
                        readonly property string actorId: actorRow.modelData && actorRow.modelData.id ? String(actorRow.modelData.id) : ""
                        function selectActor() {
                            if (root.session)
                                root.session.selectedActorId = actorRow.actorId
                        }
                        width: actorColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: root.session && actorRow.actorId === String(root.session.selectedActorId || "")
                               ? root.theme.selectionHighlight
                               : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "actorSidebarMouse_" + actorRow.actorId
                            anchors.fill: parent
                            preventStealing: true
                            onClicked: actorRow.selectActor()
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                id: actorNameText
                                objectName: "actorSidebarName_" + actorRow.actorId
                                width: parent.width
                                text: actorRow.modelData.name ? actorRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                        }
                    }
                }
            }
        }
    }
}
