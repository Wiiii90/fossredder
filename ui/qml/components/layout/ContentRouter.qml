import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: contentRouter
    Layout.fillWidth: true
    Layout.fillHeight: true

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "qrc:/qml/components/layout/Content.qml"
        asynchronous: false
        onLoaded: updateContent()
    }

    function updateContent() {
        if (!mainLoader.item) return
        var s = (uiNav && uiNav.section === UiNavigation.Actors)
                    ? "qrc:/qml/views/ActorsView.qml"
                    : (uiNav && uiNav.section === UiNavigation.Properties)
                        ? "qrc:/qml/views/PropertiesView.qml"
                        : (uiNav && uiNav.section === UiNavigation.Contracts)
                            ? "qrc:/qml/views/ContractsView.qml"
                            : (uiNav && uiNav.section === UiNavigation.Booking)
                                ? "qrc:/qml/views/BookingView.qml"
        : (uiNav && uiNav.section === UiNavigation.Import)
                                    ? "qrc:/qml/views/ImportView.qml"
                                    : (uiNav && uiNav.section === UiNavigation.Export)
                                        ? "qrc:/qml/views/ExportView.qml"
                                        : (uiNav && uiNav.section === UiNavigation.Analysis)
                                            ? "qrc:/qml/views/AnalysisView.qml"
                                            : (uiNav && uiNav.section === UiNavigation.Annual)
                                                ? "qrc:/qml/views/AnnualView.qml"
                                                : "qrc:/qml/views/PlaceholderView.qml";
        try { mainLoader.item.setContentSource(s) } catch(e) {}
    }

    Connections { target: uiNav; function onSectionChanged() { updateContent() } }
}
