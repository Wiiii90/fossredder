import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

MenuBar {
    id: root

    function clearBookingStateForSection(section) {
        if (!navigation || section === Navigation.Booking) return
        navigation.bookingView = Navigation.Statements
    }

    function navigateToSection(section) {
        if (!navigation) return
        clearBookingStateForSection(section)
        navigation.section = section
    }

    background: Rectangle {
        color: Theme.toolbarBackground
        border.width: Theme.borderWidthThin
        border.color: Theme.toolbarBorder
    }

    Menu {
        title: qsTr("File")

        Action {
            text: qsTr("New")
            shortcut: StandardKey.New
            onTriggered: if (actions) actions.newFile()
        }

        Action {
            text: qsTr("Open...")
            shortcut: StandardKey.Open
            onTriggered: if (actions) actions.openFile()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Save")
            shortcut: StandardKey.Save
            onTriggered: if (actions) actions.saveFile()
        }

        Action {
            text: qsTr("Save As...")
            shortcut: StandardKey.SaveAs
            onTriggered: if (actions) actions.saveFileAs()
        }

        MenuSeparator { }

            Action {
                text: qsTr("Import...")
                onTriggered: {
                    navigateToSection(Navigation.Import)
                    if (actions) actions.browseImportPdf()
                }
            }

            Action {
                text: qsTr("Export...")
                onTriggered: {
                    navigateToSection(Navigation.Export)
                }
            }

        MenuSeparator { }

            Action {
                text: qsTr("Quit")
                shortcut: StandardKey.Quit
                onTriggered: {
                    if (actions && actions.quitAction) actions.quitAction.trigger()
                    else Qt.quit()
                }
            }
    }

    Menu {
        title: qsTr("View")

        Action {
            text: qsTr("Import")
            onTriggered: navigateToSection(Navigation.Import)
        }
        Action {
            text: qsTr("Export")
            onTriggered: navigateToSection(Navigation.Export)
        }
        Action {
            text: qsTr("Booking")
            onTriggered: navigateToSection(Navigation.Booking)
        }
        Action {
            text: qsTr("Actors")
            onTriggered: navigateToSection(Navigation.Actors)
        }
        Action {
            text: qsTr("Properties")
            onTriggered: navigateToSection(Navigation.Properties)
        }
        Action {
            text: qsTr("Contracts")
            onTriggered: navigateToSection(Navigation.Contracts)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Analysis")
            onTriggered: navigateToSection(Navigation.Analysis)
        }
        Action {
            text: qsTr("Annual")
            onTriggered: navigateToSection(Navigation.Annual)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Settings")
            onTriggered: navigateToSection(Navigation.Settings)
        }
    }

    Menu {
        id: languageMenu
        title: qsTr("Language")

        Instantiator {
            model: languageController ? languageController.availableLanguages : []

            delegate: Action {
                required property var modelData
                text: modelData.label
                checkable: true
                checked: languageController && languageController.currentLanguage === modelData.code
                enabled: modelData.available !== false
                onTriggered: {
                    if (languageController && enabled) languageController.currentLanguage = modelData.code
                }
            }

            onObjectAdded: function(index, object) { languageMenu.insertAction(index, object) }
            onObjectRemoved: function(index, object) { languageMenu.removeAction(object) }
        }
    }

    Menu {
        title: qsTr("Help")

        Action {
            text: qsTr("About")
            onTriggered: {
                if (actions && actions.aboutAction) actions.aboutAction.trigger()
            }
        }
    }
}
