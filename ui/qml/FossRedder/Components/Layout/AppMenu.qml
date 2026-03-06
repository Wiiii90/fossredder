import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

MenuBar {
    id: root

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
            onTriggered: if (uiActions) uiActions.newFile()
        }

        Action {
            text: qsTr("Open...")
            shortcut: StandardKey.Open
            onTriggered: if (uiActions) uiActions.openFile()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Save")
            shortcut: StandardKey.Save
            onTriggered: if (uiActions) uiActions.saveFile()
        }

        Action {
            text: qsTr("Save As...")
            shortcut: StandardKey.SaveAs
            onTriggered: if (uiActions) uiActions.saveFileAs()
        }

        MenuSeparator { }

        Action {
            text: qsTr("Import...")
            onTriggered: {
                if (uiNav) uiNav.section = UiNavigation.Import
                if (uiActions) uiActions.browseImportPdf()
            }
        }

        Action {
            text: qsTr("Export...")
            onTriggered: {
                if (uiNav) uiNav.section = UiNavigation.Export
            }
        }

        MenuSeparator { }

        Action {
            text: qsTr("Quit")
            shortcut: StandardKey.Quit
            onTriggered: {
                if (uiActions && uiActions.quitAction) uiActions.quitAction.trigger()
                else Qt.quit()
            }
        }
    }

    Menu {
        title: qsTr("View")

        Action {
            text: qsTr("Import")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Import
        }
        Action {
            text: qsTr("Export")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Export
        }
        Action {
            text: qsTr("Booking")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Booking
        }
        Action {
            text: qsTr("Actors")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Actors
        }
        Action {
            text: qsTr("Properties")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Properties
        }
        Action {
            text: qsTr("Contracts")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Contracts
        }
        MenuSeparator { }
        Action {
            text: qsTr("Analysis")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Analysis
        }
        Action {
            text: qsTr("Annual")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Annual
        }
        MenuSeparator { }
        Action {
            text: qsTr("Settings")
            onTriggered: if (uiNav) uiNav.section = UiNavigation.Settings
        }
    }

    Menu {
        title: qsTr("Help")

        Action {
            text: qsTr("About")
            onTriggered: {
                if (uiActions && uiActions.aboutAction) uiActions.aboutAction.trigger()
            }
        }
    }
}
