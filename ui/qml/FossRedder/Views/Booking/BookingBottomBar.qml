/**
 * @file ui/qml/FossRedder/Views/Booking/BookingBottomBar.qml
 * @brief Provides the Booking bottom action bar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme
    required property var bookingState

    implicitWidth: bar.implicitWidth
    implicitHeight: bar.implicitHeight

    Components.BottomBar {
        id: bar
        anchors.fill: root
        theme: root.theme

        Controls.PrevPageButton {
            objectName: "bookingPreviousStatementButton"
            enabled: root.bookingState.hasStatements
            onClicked: root.bookingState.previousStatement()
        }

        Controls.PrevButton {
            objectName: "bookingPreviousTransactionButton"
            enabled: root.bookingState.hasMultipleTransactions
            onClicked: root.bookingState.previousTransaction()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.DangerButton {
            objectName: "bookingClearButton"
            visible: root.bookingState.isCreateMode
            text: qsTr("Clear")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.bookingState.resetCreateState()
        }

        Controls.SuccessButton {
            objectName: "bookingCreateButton"
            visible: root.bookingState.isCreateMode
            text: qsTr("Create")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            enabled: root.bookingState.canCreate
            onClicked: root.bookingState.submit()
        }

        Controls.DangerButton {
            objectName: "bookingDeleteButton"
            visible: !root.bookingState.isCreateMode
            text: qsTr("Delete")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            onClicked: root.bookingState.deleteCurrentStatement()
        }

        Controls.SuccessButton {
            objectName: "bookingUpdateButton"
            visible: !root.bookingState.isCreateMode
            text: qsTr("Update")
            Layout.preferredWidth: root.theme.viewActionButtonWidth
            enabled: root.bookingState.canUpdate
            onClicked: root.bookingState.updateCurrent()
        }

        Item {
            Layout.fillWidth: true
        }

        Controls.NextButton {
            objectName: "bookingNextTransactionButton"
            enabled: root.bookingState.hasMultipleTransactions
            onClicked: root.bookingState.nextTransaction()
        }

        Controls.NextPageButton {
            objectName: "bookingNextStatementButton"
            enabled: root.bookingState.hasStatements
            onClicked: root.bookingState.nextStatement()
        }
    }
}
