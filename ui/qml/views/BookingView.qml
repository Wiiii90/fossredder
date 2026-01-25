import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    id: root

    Loader {
        anchors.fill: parent
        source: (uiNav && uiNav.bookingView === UiNavigation.Statements)
                    ? ((uiData && uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                            ? "qrc:/qml/views/BookingTransactionsView.qml"
                            : "qrc:/qml/views/BookingStatementsView.qml")
                    : "qrc:/qml/views/PlaceholderView.qml"
    }
}
