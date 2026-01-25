import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: sidebarLeft

    // Local enum values matching UiNavigation Section order
    readonly property int sectionActors: 0
    readonly property int sectionProperties: 1
    readonly property int sectionContracts: 2
    readonly property int sectionBooking: 3
    readonly property int sectionImport: 4

    property int resolvedSection: -1
    property string lastSourceTried: ""

    function srcForSection(sec) {
        switch (sec) {
        case sectionActors: return "qrc:/qml/views/ActorsSidebar.qml";
        case sectionProperties: return "qrc:/qml/views/PropertiesSidebar.qml";
        case sectionContracts: return "qrc:/qml/views/ContractsSidebar.qml";
        case sectionBooking: return "qrc:/qml/views/BookingSidebar.qml";
        case sectionImport: return "qrc:/qml/views/ImportSidebar.qml";
        default: return "qrc:/qml/views/PlaceholderSidebar.qml";
        }
    }

    function setWrapperContent(src) {
        if (!sidebarLoader.item) return;
        lastSourceTried = src;
        try {
            if (typeof sidebarLoader.item.setContentSource === 'function') {
                sidebarLoader.item.setContentSource(src);
                return;
            }
        } catch (e) { }

        // fallback: try to access inner loader directly
        try {
            if (sidebarLoader.item.content) {
                sidebarLoader.item.content.source = src;
            }
        } catch (e) { }
    }

    function resolveSection() {
        if (!uiNav) return sectionImport;
        var v = uiNav.section;
        if (typeof v === 'number') return v;
        try {
            // if enum object provided stringify and match
            var s = String(v);
            if (s.indexOf('Actors') >= 0) return sectionActors;
            if (s.indexOf('Properties') >= 0) return sectionProperties;
            if (s.indexOf('Contracts') >= 0) return sectionContracts;
            if (s.indexOf('Booking') >= 0) return sectionBooking;
            if (s.indexOf('Import') >= 0) return sectionImport;
        } catch (e) {}
        return sectionImport;
    }

    function update() {
        var sec = resolveSection();
        resolvedSection = sec;
        if (!sidebarLoader.item) return;
        setWrapperContent(srcForSection(sec));
    }

    Loader {
        id: sidebarLoader
        anchors.fill: parent
        asynchronous: false
        source: "qrc:/qml/components/layout/Sidebar.qml"
        onLoaded: {
            if (!sidebarLoader.item) return;
            // header intentionally removed in wrapper
            update();
            // schedule a retry in case context properties arrive slightly later
            retryTimer.start();
        }
    }

    Connections { target: uiNav; function onSectionChanged() { update(); } }

    Timer {
        id: retryTimer
        interval: 500
        repeat: false
        onTriggered: update()
    }
}
