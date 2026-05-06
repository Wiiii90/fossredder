/**
 * @file ui/qml/FossRedder/Views/Property/PropertySidebar.qml
 * @brief Shows and selects property entries in the sidebar list.
 */

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
            contentHeight: propertyColumn.implicitHeight

            Column {
                id: propertyColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.session ? root.session.properties : []

                    delegate: Rectangle { id: propertyRow
                        required property var modelData
                        width: propertyColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: root.session && propertyRow.modelData.id === root.session.selectedPropertyId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session) root.session.selectedPropertyId = propertyRow.modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                width: parent.width
                                text: propertyRow.modelData.name ? propertyRow.modelData.name : ""
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
