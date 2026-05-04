/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionAllocatablePanel.qml
 * @brief Provides the BookingTransactionAllocatablePanel component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var txRoot

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

            Rectangle {
                anchors.fill: parent
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Text {
                anchors.centerIn: parent
                text: root.txRoot.allocatableValue() ? qsTr("Allocatable") : qsTr("Not allocatable")
                color: root.theme.textPrimary
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.fontSize
                font.bold: false
                font.weight: Font.Normal
            }

            MouseArea {
                objectName: "bookingTransactionAllocatableToggle"
                anchors.fill: parent
                onClicked: root.txRoot.updateField("allocatable", !root.txRoot.allocatableValue())
            }
        }
    }
}
