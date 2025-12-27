import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: contentArea
    Layout.fillWidth: true
    Layout.fillHeight: true

    Loader {
        anchors.fill: parent
        source: (uiNav && uiNav.section === UiNavigation.Actors)
                    ? "views/ActorsView.qml"
                    : (uiNav && uiNav.section === UiNavigation.Properties)
                        ? "views/PropertiesView.qml"
                        : (uiNav && uiNav.section === UiNavigation.Contracts)
                            ? "views/ContractsView.qml"
                            : "views/PlaceholderView.qml"
    }
}
