/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Booking/BookingTransactionPropertyPanel.qml
 * @brief Provides the BookingTransactionPropertyPanel component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionPropertyPanel.qml
 * @brief Property multi-selection panel for a transaction in the booking page.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

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

        Label {
            text: qsTr("Property")
            Layout.fillWidth: true
        }

        Repeater {
            model: root.txRoot.propertyRows || []

            delegate: RowLayout {
                id: propertyDelegate
                required property var modelData
                readonly property string propertyId: propertyDelegate.modelData && propertyDelegate.modelData.id ? propertyDelegate.modelData.id : ""
                readonly property string propertyLabel: propertyDelegate.modelData && propertyDelegate.modelData.display
                    ? propertyDelegate.modelData.display
                    : (propertyDelegate.modelData && propertyDelegate.modelData.name ? propertyDelegate.modelData.name : "")

                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Controls.CheckBox {
                    Layout.fillWidth: false
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    checked: root.txRoot.hasProperty(propertyDelegate.propertyId)
                    onClicked: root.txRoot.toggleProperty(propertyDelegate.propertyId, checked)
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
