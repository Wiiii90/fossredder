import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    id: root

    Loader {
        anchors.fill: parent
        source: (uiNav && uiNav.bookingView === UiNavigation.Statements)
                    ? ((uiData && uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                            ? "BookingTransactionsView.qml"
                            : "BookingStatementsView.qml")
                    : "PlaceholderView.qml"
    }
}
