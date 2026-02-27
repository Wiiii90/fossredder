import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Views 1.0 as Views

Item {
    id: contentRouter
    Layout.fillWidth: true
    Layout.fillHeight: true

    Content {
        id: content
        anchors.fill: parent
    }

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

    function updateContent() {
        if (!content) return

        var comp = (uiNav && uiNav.section === UiNavigation.Actors)
                    ? actorViewComp
                    : (uiNav && uiNav.section === UiNavigation.Properties)
                        ? propertyViewComp
                        : (uiNav && uiNav.section === UiNavigation.Contracts)
                            ? contractViewComp
                            : (uiNav && uiNav.section === UiNavigation.Booking)
                                ? bookingViewComp
                                : (uiNav && uiNav.section === UiNavigation.Import)
                                    ? importViewComp
                                    : (uiNav && uiNav.section === UiNavigation.Export)
                                        ? exportViewComp
                                        : (uiNav && uiNav.section === UiNavigation.Settings)
                                            ? settingsViewComp
                                            : (uiNav && uiNav.section === UiNavigation.Analysis)
                                                ? analysisViewComp
                                                : (uiNav && uiNav.section === UiNavigation.Annual)
                                                    ? annualViewComp
                                                    : placeholderViewComp;

        content.setContentComponent(comp)
    }

    Component.onCompleted: updateContent()
    Connections { target: uiNav; function onSectionChanged() { updateContent() } }
}

