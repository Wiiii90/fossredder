/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionAllocatablePanel.qml
 * @brief Provides the BookingTransactionAllocatablePanel component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var bookingState

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
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Item {
            id: toggleSurface
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.controlHeight

            Rectangle {
                anchors.fill: toggleSurface
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Text {
                anchors.centerIn: toggleSurface
                text: root.bookingState.transactionAllocatable ? qsTr("Allocatable") : qsTr("Not allocatable")
                color: root.theme.textPrimary
                font.family: root.theme.fontFamily
                font.pointSize: root.theme.fontSize
                font.bold: false
                font.weight: Font.Normal
            }

            MouseArea {
                objectName: "bookingTransactionAllocatableToggle"
                anchors.fill: toggleSurface
                onClicked: root.bookingState.transactionAllocatable = !root.bookingState.transactionAllocatable
            }
        }
    }
}
