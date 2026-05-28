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

    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property var propertyState: root.workspaceFacade ? root.workspaceFacade.propertyState : null
    readonly property var propertyRows: root.workspaceFacade ? root.workspaceFacade.propertyRows : []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            objectName: "propertySidebarFlick"
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
                    model: root.propertyRows

                    delegate: Rectangle {
                        id: propertyRow
                        objectName: "propertySidebarRow_" + (propertyRow.modelData && propertyRow.modelData.id ? String(propertyRow.modelData.id) : "")
                        required property var modelData
                        readonly property string propertyId: propertyRow.modelData && propertyRow.modelData.id ? String(propertyRow.modelData.id) : ""
                        width: propertyColumn.width
                        height: root.theme.viewSidebarRowHeight
                        radius: root.theme.viewSidebarRowRadius
                        color: root.workspaceFacade && propertyRow.propertyId === root.workspaceFacade.selectedPropertyId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            objectName: "propertySidebarMouse_" + propertyRow.propertyId
                            anchors.fill: parent
                            onClicked: if (root.propertyState) root.propertyState.selectProperty(propertyRow.propertyId)
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: root.theme.viewSidebarRowSpacing

                            Text {
                                objectName: "propertySidebarName_" + propertyRow.propertyId
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
