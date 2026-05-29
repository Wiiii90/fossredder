/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementForm.qml
 * @brief Provides the Booking statement form composition.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views.Booking 1.0 as Booking
pragma ComponentBehavior: Bound

Item {
    id: root

    required property var theme
    required property var bookingState
    property bool readOnly: false

    Layout.fillWidth: true
    Layout.fillHeight: true

    ColumnLayout {
        anchors.fill: root
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Statement")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Controls.TextField {
                objectName: "bookingStatementNameField"
                Layout.fillWidth: true
                readOnly: root.readOnly
                text: root.bookingState.statementName
                onTextEdited: root.bookingState.statementName = text
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                objectName: "bookingTransactionInfoLabel"
                text: root.bookingState.transactionInfoText
                color: root.theme.textMuted
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.CompactAddButton {
                objectName: "bookingStatementAddTransactionButton"
                visible: root.bookingState.canAddTransaction
                enabled: root.bookingState.canAddTransaction
                onClicked: root.bookingState.addTransactionAfterCurrent()
            }

            Controls.CompactRemoveButton {
                objectName: "bookingStatementRemoveTransactionButton"
                visible: true
                enabled: root.bookingState.canDeleteTransaction
                onClicked: root.bookingState.deleteCurrentTransaction()
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentSpacing: 0
            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Flickable {
                id: transactionScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: contentLayout.implicitHeight

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                ColumnLayout {
                    id: contentLayout
                    width: transactionScroll.width
                    spacing: root.theme.spacingSmall

                    Booking.BookingTransactionView {
                        Layout.fillWidth: true
                        theme: root.theme
                        bookingState: root.bookingState
                    }
                }
            }
        }
    }
}
