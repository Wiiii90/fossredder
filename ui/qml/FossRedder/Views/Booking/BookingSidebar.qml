import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: root

    Component { id: bookingStatementsSidebarComp; Views.BookingStatementsSidebar { } }
    Component { id: placeholderSidebarComp; Views.PlaceholderSidebar { } }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("Statements")
                checkable: true
                checked: navigation && navigation.bookingView === Navigation.Statements
                onClicked: if (navigation) navigation.bookingView = Navigation.Statements
            }
            Button {
                text: qsTr("Calendar")
                checkable: true
                checked: navigation && navigation.bookingView === Navigation.Calendar
                onClicked: if (navigation) navigation.bookingView = Navigation.Calendar
            }
            Button {
                text: qsTr("Transactions")
                checkable: true
                checked: navigation && navigation.bookingView === Navigation.Transactions
                onClicked: if (navigation) navigation.bookingView = Navigation.Transactions
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: (navigation && navigation.bookingView === Navigation.Statements)
                             ? bookingStatementsSidebarComp
                             : placeholderSidebarComp
        }
    }
}

