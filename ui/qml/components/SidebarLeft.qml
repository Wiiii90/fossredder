import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: sidebarLeft

    Loader {
        anchors.fill: parent
        source: (uiNav && uiNav.section === UiNavigation.Actors)
                    ? "views/ActorsSidebar.qml"
                    : (uiNav && uiNav.section === UiNavigation.Properties)
                        ? "views/PropertiesSidebar.qml"
                        : (uiNav && uiNav.section === UiNavigation.Contracts)
                            ? "views/ContractsSidebar.qml"
                            : "views/PlaceholderSidebar.qml"
    }
}
