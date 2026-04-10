import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: contentRouter
    Layout.fillWidth: true
    Layout.fillHeight: true
    property int activeSection: navigation ? navigation.section : Navigation.Import
    property bool actorLoaded: false
    property bool propertyLoaded: false
    property bool contractLoaded: false
    property bool bookingLoaded: false
    property bool importLoaded: false
    property bool exportLoaded: false
    property bool settingsLoaded: false
    property bool analysisLoaded: false
    property bool annualLoaded: false
    property bool placeholderLoaded: false

    Component { id: actorViewComp; Views.ActorView { } }
    Component { id: propertyViewComp; Views.PropertyView { } }
    Component { id: contractViewComp; Views.ContractView { } }
    Component { id: bookingViewComp; Views.BookingView { } }
    Component { id: importViewComp; Views.ImportView { } }
    Component { id: exportViewComp; Views.ExportView { } }
    Component { id: settingsViewComp; Views.SettingsView { } }
    Component { id: analysisViewComp; Views.AnalysisView { } }
    Component { id: annualViewComp; Views.AnnualView { } }
    Component { id: placeholderViewComp; Views.PlaceholderView { } }

    function rememberSection(section) {
        switch (section) {
        case Navigation.Actors:
            actorLoaded = true
            break
        case Navigation.Properties:
            propertyLoaded = true
            break
        case Navigation.Contracts:
            contractLoaded = true
            break
        case Navigation.Booking:
            bookingLoaded = true
            break
        case Navigation.Import:
            importLoaded = true
            break
        case Navigation.Export:
            exportLoaded = true
            break
        case Navigation.Settings:
            settingsLoaded = true
            break
        case Navigation.Analysis:
            analysisLoaded = true
            break
        case Navigation.Annual:
            annualLoaded = true
            break
        default:
            placeholderLoaded = true
            break
        }
    }

    function updateContent() {
        if (navigation && navigation.section !== Navigation.Booking && session) {
            if (session.selectedTransactionId && session.selectedTransactionId.length > 0)
                session.selectedTransactionId = ""
            if (session.selectedStatementId && session.selectedStatementId.length > 0)
                session.selectedStatementId = ""
        }

        if (navigation && navigation.section !== Navigation.Booking && navigation.bookingView !== Navigation.Statements)
            navigation.bookingView = Navigation.Statements

        activeSection = navigation ? navigation.section : Navigation.Import
        rememberSection(activeSection)
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Actors
        contentComponent: actorLoaded ? actorViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Properties
        contentComponent: propertyLoaded ? propertyViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Contracts
        contentComponent: contractLoaded ? contractViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Booking
        contentComponent: bookingLoaded ? bookingViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Import
        contentComponent: importLoaded ? importViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Export
        contentComponent: exportLoaded ? exportViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Settings
        contentComponent: settingsLoaded ? settingsViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Analysis
        contentComponent: analysisLoaded ? analysisViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === Navigation.Annual
        contentComponent: annualLoaded ? annualViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection !== Navigation.Actors
                 && activeSection !== Navigation.Properties
                 && activeSection !== Navigation.Contracts
                 && activeSection !== Navigation.Booking
                 && activeSection !== Navigation.Import
                 && activeSection !== Navigation.Export
                 && activeSection !== Navigation.Settings
                 && activeSection !== Navigation.Analysis
                 && activeSection !== Navigation.Annual
        contentComponent: placeholderLoaded ? placeholderViewComp : null
    }

    Component.onCompleted: updateContent()
    Connections { target: navigation; function onSectionChanged() { updateContent() } }
}

