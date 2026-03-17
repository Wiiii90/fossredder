import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: root
    property Component pendingViewComponent: null
    property Component currentViewComponent: null

    Component { id: bookingTransactionsComp; Views.BookingTransactionsView { } }
    Component { id: bookingStatementsComp; Views.BookingStatementsView { } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    function resolveViewComponent() {
        if (!uiNav)
            return placeholderViewComp

        if (uiNav.bookingView === UiNavigation.Transactions)
            return bookingTransactionsComp

        if (uiNav.bookingView === UiNavigation.Statements)
            return (uiData && uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                    ? bookingTransactionsComp
                    : bookingStatementsComp

        return placeholderViewComp
    }

    function queueResolvedView() {
        root.pendingViewComponent = resolveViewComponent()
        applyBookingViewTimer.restart()
    }

    Timer {
        id: applyBookingViewTimer
        interval: 0
        repeat: false
        onTriggered: {
            root.currentViewComponent = root.pendingViewComponent
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: root.currentViewComponent
    }

    Connections {
        target: uiNav
        function onBookingViewChanged() {
            root.queueResolvedView()
        }
    }

    Connections {
        target: uiData
        function onSelectedTransactionIdChanged() {
            root.queueResolvedView()
        }
    }

    Component.onCompleted: {
        queueResolvedView()
    }
}

