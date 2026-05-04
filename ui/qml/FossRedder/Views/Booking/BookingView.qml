/**
 * @file ui/qml/FossRedder/Views/Booking/BookingView.qml
 * @brief Provides the BookingView component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.BookingStatementView {
        anchors.fill: parent
        appContext: root.appContext
        theme: root.theme
    }
}

