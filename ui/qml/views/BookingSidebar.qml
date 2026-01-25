import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("Statements")
                checkable: true
                checked: uiNav && uiNav.bookingView === UiNavigation.Statements
                onClicked: if (uiNav) uiNav.bookingView = UiNavigation.Statements
            }
            Button {
                text: qsTr("Calendar")
                checkable: true
                checked: uiNav && uiNav.bookingView === UiNavigation.Calendar
                onClicked: if (uiNav) uiNav.bookingView = UiNavigation.Calendar
            }
            Button {
                text: qsTr("Transactions")
                checkable: true
                checked: uiNav && uiNav.bookingView === UiNavigation.Transactions
                onClicked: if (uiNav) uiNav.bookingView = UiNavigation.Transactions
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: (uiNav && uiNav.bookingView === UiNavigation.Statements)
                        ? "qrc:/qml/views/BookingStatementsSidebar.qml"
                        : "qrc:/qml/views/PlaceholderSidebar.qml"
        }
    }
}
