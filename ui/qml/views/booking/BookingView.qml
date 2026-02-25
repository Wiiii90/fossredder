import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    id: root

    Loader {
        anchors.fill: parent
        source: (uiNav && uiNav.bookingView === UiNavigation.Statements)
                    ? ((uiData && uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                            ? "qrc:/qml/views/booking/BookingTransactionsView.qml"
                            : "qrc:/qml/views/booking/BookingStatementsView.qml")
                    : "qrc:/qml/views/PlaceholderView.qml"
    }
}
