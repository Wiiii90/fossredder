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
        case Navigation.Export:
            exportLoaded = true
            break
        case Navigation.Analysis:
            analysisLoaded = true
            break
        case Navigation.Annual:
            annualLoaded = true
            break
        case Navigation.Settings:
            settingsLoaded = true
            break
        default:
            placeholderLoaded = true
            break
        }
    }

    function resolveSection() {
        if (!AppContext.navigation) return sectionImport;
        var v = AppContext.navigation.section;
        try {
            if (typeof v === 'string' && v.indexOf('Export') >= 0) return Navigation.Export;
        } catch(e) {}
        if (typeof v === 'number') return v;
        try {
            var s = String(v);
            if (s.indexOf('Actors') >= 0) return sectionActors;
            if (s.indexOf('Properties') >= 0) return sectionProperties;
            if (s.indexOf('Contracts') >= 0) return sectionContracts;
            if (s.indexOf('Booking') >= 0) return sectionBooking;
            if (s.indexOf('Import') >= 0) return sectionImport;
            if (s.indexOf('Export') >= 0) return Navigation.Export;
        } catch (e) {}
        return sectionImport;
    }

    function update() {
        var sec = resolveSection();

        if (sec !== sectionBooking && AppContext.session) {
            if (AppContext.session.selectedTransactionId && AppContext.session.selectedTransactionId.length > 0)
                AppContext.session.selectedTransactionId = ""
            if (AppContext.session.selectedStatementId && AppContext.session.selectedStatementId.length > 0)
                AppContext.session.selectedStatementId = ""
        }

        if (sec !== sectionBooking && AppContext.navigation && AppContext.navigation.bookingView !== Navigation.Statements)
            AppContext.navigation.bookingView = Navigation.Statements

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
        visible: resolvedSection === Navigation.Export
        contentComponent: exportLoaded ? exportSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === Navigation.Analysis
        contentComponent: analysisLoaded ? analysisSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === Navigation.Annual
        contentComponent: annualLoaded ? annualSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection === Navigation.Settings
        contentComponent: settingsLoaded ? settingsSidebarComp : null
    }

    Sidebar {
        anchors.fill: parent
        visible: resolvedSection !== sectionActors
                 && resolvedSection !== sectionProperties
                 && resolvedSection !== sectionContracts
                 && resolvedSection !== sectionBooking
                 && resolvedSection !== sectionImport
                 && resolvedSection !== Navigation.Export
                 && resolvedSection !== Navigation.Analysis
                 && resolvedSection !== Navigation.Annual
                 && resolvedSection !== Navigation.Settings
        contentComponent: placeholderLoaded ? placeholderSidebarComp : null
    }

    Connections { target: AppContext.navigation; function onSectionChanged() { update(); } }

    Component.onCompleted: {
        update()
    }
}

