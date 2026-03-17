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
    property bool actorLoaded: false
    property bool propertyLoaded: false
    property bool contractLoaded: false
    property bool bookingLoaded: false
    property bool importLoaded: false
    property bool exportLoaded: false
    property bool analysisLoaded: false
    property bool annualLoaded: false
    property bool settingsLoaded: false
    property bool placeholderLoaded: false

    function rememberSection(sec) {
        switch (sec) {
        case sectionActors:
            actorLoaded = true
            break
        case sectionProperties:
            propertyLoaded = true
            break
        case sectionContracts:
            contractLoaded = true
            break
        case sectionBooking:
            bookingLoaded = true
            break
        case sectionImport:
            importLoaded = true
            break
        case UiNavigation.Export:
            exportLoaded = true
            break
        case UiNavigation.Analysis:
            analysisLoaded = true
            break
        case UiNavigation.Annual:
            annualLoaded = true
            break
        case UiNavigation.Settings:
            settingsLoaded = true
            break
        default:
            placeholderLoaded = true
            break
        }
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

        if (sec !== sectionBooking && uiData) {
            if (uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                uiData.selectedTransactionId = ""
            if (uiData.selectedStatementId && uiData.selectedStatementId.length > 0)
                uiData.selectedStatementId = ""
        }

        if (sec !== sectionBooking && uiNav && uiNav.bookingView !== UiNavigation.Statements)
            uiNav.bookingView = UiNavigation.Statements

        resolvedSection = sec;
        rememberSection(sec)
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

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === sectionActors
        contentComponent: actorLoaded ? actorSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === sectionProperties
        contentComponent: propertyLoaded ? propertySidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === sectionContracts
        contentComponent: contractLoaded ? contractSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === sectionBooking
        contentComponent: bookingLoaded ? bookingSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === sectionImport
        contentComponent: importLoaded ? importSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === UiNavigation.Export
        contentComponent: exportLoaded ? exportSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === UiNavigation.Analysis
        contentComponent: analysisLoaded ? analysisSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === UiNavigation.Annual
        contentComponent: annualLoaded ? annualSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === UiNavigation.Settings
        contentComponent: settingsLoaded ? settingsSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection !== sectionActors
                 && resolvedSection !== sectionProperties
                 && resolvedSection !== sectionContracts
                 && resolvedSection !== sectionBooking
                 && resolvedSection !== sectionImport
                 && resolvedSection !== UiNavigation.Export
                 && resolvedSection !== UiNavigation.Analysis
                 && resolvedSection !== UiNavigation.Annual
                 && resolvedSection !== UiNavigation.Settings
        contentComponent: placeholderLoaded ? placeholderSidebarComp : null
    }

    Connections { target: uiNav; function onSectionChanged() { update(); } }

    Component.onCompleted: {
        update()
    }
}

