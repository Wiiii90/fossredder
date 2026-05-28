/**
 * @file ui/qml/FossRedder/Views/Booking/BookingSidebar.qml
 * @brief Provides the BookingSidebar component.
 */

import QtQuick 2.15
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
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: bookingStatementsSidebarComp
        }
    }
}
