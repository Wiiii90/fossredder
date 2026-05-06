/**
 * @file ui/qml/FossRedder/Components/Layout/AppMenu.qml
 * @brief Provides the AppMenu component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
pragma ComponentBehavior: Bound

MenuBar {
    id: root
    required property var navigation
    required property var actions
    required property var languageController
    required property var settingsController
    required property var theme
    readonly property int navActors: 0
    readonly property int navProperties: 1
    readonly property int navContracts: 2
    readonly property int navBooking: 3
    readonly property int navImport: 4
    readonly property int navExport: 5
    readonly property int navSettings: 6
    readonly property int navAnalysis: 7
    readonly property int navAnnual: 8
    readonly property int navStatements: 0

    function clearBookingStateForSection(section) {
        if (!root.navigation || section === root.navBooking) return
    }

    function navigateToSection(section) {
        if (!root.navigation) return
        root.clearBookingStateForSection(section)
        root.navigation.setSectionValue(section)
    }

    background: Rectangle {
        color: root.theme.toolbarBackground
        border.width: root.theme.borderWidthThin
        border.color: root.theme.toolbarBorder
    }

    Menu {
        title: qsTr("File")

        Action {
            text: qsTr("New")
            shortcut: StandardKey.New
            onTriggered: if (root.actions) root.actions.newFile()
        }

        Action {
            text: qsTr("Open...")
            shortcut: StandardKey.Open
            onTriggered: if (root.actions) root.actions.openFile()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Save")
            shortcut: StandardKey.Save
            onTriggered: if (root.actions) root.actions.saveFile()
        }

        Action {
            text: qsTr("Save As...")
            shortcut: StandardKey.SaveAs
            onTriggered: if (root.actions) root.actions.saveFileAs()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Import...")
            onTriggered: {
                root.navigateToSection(root.navImport)
                if (root.actions) root.actions.browseImportPdf()
            }
        }

        Action {
            text: qsTr("Export...")
            onTriggered: {
                root.navigateToSection(root.navExport)
            }
        }

        MenuSeparator { }

        Action {
            text: qsTr("Quit")
            shortcut: StandardKey.Quit
            onTriggered: {
                if (root.actions && root.actions.quitAction) root.actions.quitAction.trigger()
                else Qt.quit()
            }
        }
    }

    Menu {
        title: qsTr("View")

        Action {
            text: qsTr("Import")
            onTriggered: root.navigateToSection(root.navImport)
        }
        Action {
            text: qsTr("Export")
            onTriggered: root.navigateToSection(root.navExport)
        }
        Action {
            text: qsTr("Booking")
            onTriggered: root.navigateToSection(root.navBooking)
        }
        Action {
            text: qsTr("Actors")
            onTriggered: root.navigateToSection(root.navActors)
        }
        Action {
            text: qsTr("Properties")
            onTriggered: root.navigateToSection(root.navProperties)
        }
        Action {
            text: qsTr("Contracts")
            onTriggered: root.navigateToSection(root.navContracts)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Analysis")
            onTriggered: root.navigateToSection(root.navAnalysis)
        }
        Action {
            text: qsTr("Annual")
            onTriggered: root.navigateToSection(root.navAnnual)
        }
        MenuSeparator { }
        Action {
            text: qsTr("Settings")
            onTriggered: root.navigateToSection(root.navSettings)
        }
    }

    Menu {
        id: languageMenu
        title: qsTr("Language")

        Instantiator {
            model: root.languageController ? root.languageController.availableLanguages : []

            delegate: Action {
                required property var modelData
                text: modelData.label
                checkable: true
                checked: root.languageController && root.languageController.currentLanguage === modelData.code
                enabled: modelData.available !== false
                onTriggered: {
                    if (!enabled)
                        return
                    if (root.settingsController)
                        root.settingsController.language = modelData.code
                    if (root.languageController)
                        root.languageController.applyLanguage(modelData.code)
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
                if (root.actions && root.actions.aboutAction) root.actions.aboutAction.trigger()
            }
        }
    }
}
