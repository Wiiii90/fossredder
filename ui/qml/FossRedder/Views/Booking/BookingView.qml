import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: root

    Component { id: bookingTransactionsComp; Views.BookingTransactionsView { } }
    Component { id: bookingStatementsComp; Views.BookingStatementsView { } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    Loader {
        anchors.fill: parent
        sourceComponent: (uiNav && uiNav.bookingView === UiNavigation.Statements)
                         ? ((uiData && uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                                ? bookingTransactionsComp
                                : bookingStatementsComp)
                         : placeholderViewComp
    }
}

