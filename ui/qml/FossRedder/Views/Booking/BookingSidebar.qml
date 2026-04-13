import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var navigation: root.appContext ? root.appContext.navigation : null

    Component { id: bookingStatementsSidebarComp; Views.BookingStatementsSidebar { appContext: root.appContext; theme: root.theme } }
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
                checked: root.navigation && root.navigation.bookingViewValue === 0
                onClicked: if (root.navigation) root.navigation.setBookingViewValue(0)
            }
            Button {
                text: qsTr("Calendar")
                checkable: true
                checked: root.navigation && root.navigation.bookingViewValue === 1
                onClicked: if (root.navigation) root.navigation.setBookingViewValue(1)
            }
            Button {
                text: qsTr("Transactions")
                checkable: true
                checked: root.navigation && root.navigation.bookingViewValue === 2
                onClicked: if (root.navigation) root.navigation.setBookingViewValue(2)
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: (root.navigation && root.navigation.bookingViewValue === 0)
                             ? bookingStatementsSidebarComp
                             : placeholderSidebarComp
        }
    }
}

