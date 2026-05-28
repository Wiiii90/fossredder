/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionPropertyPanel.qml
 * @brief Provides the BookingTransactionPropertyPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var bookingState
    readonly property var selectedPropertyIds: root.bookingState.selectedPropertyIds

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
        Layout.fillHeight: true
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Property")
            Layout.fillWidth: true
        }

        Repeater {
            model: root.bookingState.propertyRows

            delegate: RowLayout {
                id: propertyDelegate
                required property var modelData
                readonly property string propertyId: propertyDelegate.modelData.id
                readonly property string propertyLabel: propertyDelegate.modelData.display

                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Controls.CheckBox {
                    objectName: "bookingTransactionPropertyCheckBox"
                    Layout.fillWidth: false
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    checked: root.selectedPropertyIds.indexOf(propertyDelegate.propertyId) !== -1
                    onToggled: root.bookingState.setPropertySelected(propertyDelegate.propertyId, checked)
                }

                Label {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    text: propertyDelegate.propertyLabel
                    elide: Text.ElideRight
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
}
