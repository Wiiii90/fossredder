import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

GridLayout {
    id: layoutRoot
    anchors.fill: parent
    columns: 1
    columnSpacing: 0
    rowSpacing: 0

    AppMenu {
        id: appMenu
        Layout.row: 0
        Layout.fillWidth: true
    }

    Toolbar {
        id: toolbar
        Layout.row: 1
        Layout.preferredHeight: toolbar.implicitHeight
        Layout.minimumHeight: toolbar.implicitHeight
        Layout.fillWidth: true
    }

    RowLayout {
        id: horizontalLayout
        Layout.row: 2
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: Theme.shellMinimumHeight
        spacing: 0

        Item {
            id: leftWrapper
            Layout.preferredWidth: Theme.shellSidebarPreferredWidth
            Layout.minimumWidth: Theme.shellSidebarMinimumWidth
            Layout.maximumWidth: Theme.shellSidebarPreferredWidth
            Layout.fillHeight: true

            SidebarRouter {
                anchors.fill: parent
            }
        }

        Item {
            id: centerWrapper
            Layout.minimumWidth: Theme.shellContentMinimumWidth
            Layout.fillWidth: true
            Layout.fillHeight: true

            ContentRouter {
                anchors.fill: parent
            }
        }
    }

    StatusBar {
        id: statusBar
        Layout.row: 3
        Layout.preferredHeight: Theme.statusBarHeight
        Layout.fillWidth: true
    }

}

