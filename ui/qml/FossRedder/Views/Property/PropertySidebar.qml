/*!
 * @file ui/qml/FossRedder/Views/Property/PropertySidebar.qml
 * @brief Sidebar list for navigating available property records.
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
                    model: root.session ? root.session.propertyRows() : []

                    delegate: Rectangle { id: propertyRow
                        required property var modelData
                        width: propertyColumn.width
                        height: 44
                        radius: 6
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
                            spacing: 2

                            Text {
                                width: parent.width
                                text: propertyRow.modelData.name ? propertyRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: propertyRow.modelData.address ? propertyRow.modelData.address : ""
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
