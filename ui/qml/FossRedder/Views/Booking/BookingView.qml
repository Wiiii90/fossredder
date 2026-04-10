import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: root
    readonly property NavigationState navigation: AppContext.navigation
    readonly property StateFacade session: AppContext.session
    property Component pendingViewComponent: null
    property Component currentViewComponent: null

    Component { id: bookingTransactionsComp; Views.BookingTransactionsView { } }
    Component { id: bookingStatementsComp; Views.BookingStatementsView { } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    function resolveViewComponent() {
        if (!navigation)
            return placeholderViewComp

        if (navigation.bookingView === Navigation.Transactions)
            return bookingTransactionsComp

        if (navigation.bookingView === Navigation.Statements)
            return (session && session.selectedTransactionId && session.selectedTransactionId.length > 0)
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
        target: navigation
        function onBookingViewChanged() {
            root.queueResolvedView()
        }
    }

    Connections {
        target: session
        function onSelectedTransactionIdChanged() {
            root.queueResolvedView()
        }
    }

    Component.onCompleted: {
        queueResolvedView()
    }
}

