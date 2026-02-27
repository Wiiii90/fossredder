import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: sidebarLeft

    readonly property int sectionActors: 0
    readonly property int sectionProperties: 1
    readonly property int sectionContracts: 2
    readonly property int sectionBooking: 3
    readonly property int sectionImport: 4

    property int resolvedSection: -1
    property string lastSourceTried: ""

    function srcForSection(sec) {
        switch (sec) {
        case sectionActors: return actorSidebarComp;
        case sectionProperties: return propertySidebarComp;
        case sectionContracts: return contractSidebarComp;
        case sectionBooking: return bookingSidebarComp;
        case sectionImport: return importSidebarComp;
        case UiNavigation.Export: return exportSidebarComp;
        case UiNavigation.Analysis: return analysisSidebarComp;
        case UiNavigation.Annual: return annualSidebarComp;
        case UiNavigation.Settings: return settingsSidebarComp;
        default: return placeholderSidebarComp;
        }
    }

    function setWrapperContent(comp) {
        if (!sidebar) return
        sidebar.setContentComponent(comp)
    }

    function resolveSection() {
        if (!uiNav) return sectionImport;
        var v = uiNav.section;
        try {
            if (typeof v === 'string' && v.indexOf('Export') >= 0) return UiNavigation.Export;
        } catch(e) {}
        if (typeof v === 'number') return v;
        try {
            var s = String(v);
            if (s.indexOf('Actors') >= 0) return sectionActors;
            if (s.indexOf('Properties') >= 0) return sectionProperties;
            if (s.indexOf('Contracts') >= 0) return sectionContracts;
            if (s.indexOf('Booking') >= 0) return sectionBooking;
            if (s.indexOf('Import') >= 0) return sectionImport;
            if (s.indexOf('Export') >= 0) return UiNavigation.Export;
        } catch (e) {}
        return sectionImport;
    }

    function update() {
        var sec = resolveSection();
        resolvedSection = sec;
        setWrapperContent(srcForSection(sec));
    }

    Sidebar {
        id: sidebar
        anchors.fill: parent
    }

    Component { id: actorSidebarComp; Views.ActorSidebar { } }
    Component { id: propertySidebarComp; Views.PropertySidebar { } }
    Component { id: contractSidebarComp; Views.ContractSidebar { } }
    Component { id: bookingSidebarComp; Views.BookingSidebar { } }
    Component { id: importSidebarComp; Views.ImportSidebar { } }
    Component { id: exportSidebarComp; Views.ExportSidebar { } }
    Component { id: analysisSidebarComp; Views.AnalysisSidebar { } }
    Component { id: annualSidebarComp; Views.AnnualSidebar { } }
    Component { id: settingsSidebarComp; Views.SettingsSidebar { } }
    Component { id: placeholderSidebarComp; Views.PlaceholderSidebar { } }

    Connections { target: uiNav; function onSectionChanged() { update(); } }

    Timer {
        id: retryTimer
        interval: 500
        repeat: false
        onTriggered: update()
    }

    Component.onCompleted: {
        update()
        retryTimer.start()
    }
}

