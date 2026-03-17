import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: contentRouter
    Layout.fillWidth: true
    Layout.fillHeight: true
    property int activeSection: uiNav ? uiNav.section : UiNavigation.Import
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
        case UiNavigation.Actors:
            actorLoaded = true
            break
        case UiNavigation.Properties:
            propertyLoaded = true
            break
        case UiNavigation.Contracts:
            contractLoaded = true
            break
        case UiNavigation.Booking:
            bookingLoaded = true
            break
        case UiNavigation.Import:
            importLoaded = true
            break
        case UiNavigation.Export:
            exportLoaded = true
            break
        case UiNavigation.Settings:
            settingsLoaded = true
            break
        case UiNavigation.Analysis:
            analysisLoaded = true
            break
        case UiNavigation.Annual:
            annualLoaded = true
            break
        default:
            placeholderLoaded = true
            break
        }
    }

    function updateContent() {
        if (uiNav && uiNav.section !== UiNavigation.Booking && uiData) {
            if (uiData.selectedTransactionId && uiData.selectedTransactionId.length > 0)
                uiData.selectedTransactionId = ""
            if (uiData.selectedStatementId && uiData.selectedStatementId.length > 0)
                uiData.selectedStatementId = ""
        }

        if (uiNav && uiNav.section !== UiNavigation.Booking && uiNav.bookingView !== UiNavigation.Statements)
            uiNav.bookingView = UiNavigation.Statements

        activeSection = uiNav ? uiNav.section : UiNavigation.Import
        rememberSection(activeSection)
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Actors
        contentComponent: actorLoaded ? actorViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Properties
        contentComponent: propertyLoaded ? propertyViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Contracts
        contentComponent: contractLoaded ? contractViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Booking
        contentComponent: bookingLoaded ? bookingViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Import
        contentComponent: importLoaded ? importViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Export
        contentComponent: exportLoaded ? exportViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Settings
        contentComponent: settingsLoaded ? settingsViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Analysis
        contentComponent: analysisLoaded ? analysisViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection === UiNavigation.Annual
        contentComponent: annualLoaded ? annualViewComp : null
    }

    Content {
        anchors.fill: parent
        visible: activeSection !== UiNavigation.Actors
                 && activeSection !== UiNavigation.Properties
                 && activeSection !== UiNavigation.Contracts
                 && activeSection !== UiNavigation.Booking
                 && activeSection !== UiNavigation.Import
                 && activeSection !== UiNavigation.Export
                 && activeSection !== UiNavigation.Settings
                 && activeSection !== UiNavigation.Analysis
                 && activeSection !== UiNavigation.Annual
        contentComponent: placeholderLoaded ? placeholderViewComp : null
    }

    Component.onCompleted: updateContent()
    Connections { target: uiNav; function onSectionChanged() { updateContent() } }
}

