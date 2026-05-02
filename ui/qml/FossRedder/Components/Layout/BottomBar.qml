/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Components/Layout/BottomBar.qml
 * @brief Provides the BottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    default property alias content: contentRow.data

    property real horizontalPadding: root.theme ? root.theme.spacing : 12
    property real verticalPadding: root.theme ? root.theme.spacing : 12

    implicitWidth: contentRow.implicitWidth + horizontalPadding * 2
    implicitHeight: contentRow.implicitHeight + verticalPadding * 2

    Rectangle {
        anchors.fill: parent
        radius: root.theme ? root.theme.radius : 3
        antialiasing: true
        color: root.theme ? root.theme.surface : "#ffffff"
        border.width: root.theme ? root.theme.borderWidthThin : 1
        border.color: root.theme ? root.theme.border : "#cccccc"

        RowLayout {
            id: contentRow
            anchors.fill: parent
            anchors.leftMargin: root.horizontalPadding
            anchors.rightMargin: root.horizontalPadding
            anchors.topMargin: root.verticalPadding
            anchors.bottomMargin: root.verticalPadding
            spacing: root.theme ? root.theme.spacingSmall : 6
        }
    }
}
