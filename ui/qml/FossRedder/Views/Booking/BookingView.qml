import QtQuick 2.15
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var navigation: root.appContext ? root.appContext.navigation : null
    readonly property var session: root.appContext ? root.appContext.session : null
    property Component pendingViewComponent: null
    property Component currentViewComponent: null

    Component { id: bookingTransactionsComp; Views.BookingTransactionsView { appContext: root.appContext; theme: root.theme } }
    Component { id: bookingStatementsComp; Views.BookingStatementsView { appContext: root.appContext } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    function resolveViewComponent() {
        if (!root.navigation)
            return placeholderViewComp

        if (root.navigation.bookingViewValue === 2)
            return bookingTransactionsComp

        if (root.navigation.bookingViewValue === 0)
            return (root.session && root.session.selectedTransactionId && root.session.selectedTransactionId.length > 0)
                    ? bookingTransactionsComp
                    : bookingStatementsComp

        return placeholderViewComp
    }

    function queueResolvedView() {
        root.pendingViewComponent = root.resolveViewComponent()
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
        target: root.navigation
        function onBookingViewChanged() {
            root.queueResolvedView()
        }
    }

    Connections {
        target: root.session
        function onSelectedTransactionIdChanged() {
            root.queueResolvedView()
        }
    }

    Component.onCompleted: {
        root.queueResolvedView()
    }
}

