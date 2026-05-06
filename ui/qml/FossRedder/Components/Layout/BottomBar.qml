/**
 * @file ui/qml/FossRedder/Components/Layout/BottomBar.qml
 * @brief Provides the BottomBar component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    default property alias content: contentRow.data

    property real horizontalPadding: root.theme && typeof root.theme.spacing === "number" ? root.theme.spacing : 12
    property real verticalPadding: root.theme && typeof root.theme.spacing === "number" ? root.theme.spacing : 12

    implicitWidth: contentRow.implicitWidth + horizontalPadding * 2
    implicitHeight: contentRow.implicitHeight + verticalPadding * 2

    Rectangle {
        anchors.fill: parent
        radius: root.theme && typeof root.theme.radius === "number" ? root.theme.radius : 3
        antialiasing: true
        color: root.theme && root.theme.surface ? root.theme.surface : "#ffffff"
        border.width: root.theme && typeof root.theme.borderWidthThin === "number" ? root.theme.borderWidthThin : 1
        border.color: root.theme && root.theme.border ? root.theme.border : "#cccccc"

        RowLayout {
            id: contentRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.fill: parent
            anchors.leftMargin: root.horizontalPadding
            anchors.rightMargin: root.horizontalPadding
            anchors.topMargin: root.verticalPadding
            anchors.bottomMargin: root.verticalPadding
            spacing: root.theme && typeof root.theme.spacingSmall === "number" ? root.theme.spacingSmall : 6
        }
    }
}
