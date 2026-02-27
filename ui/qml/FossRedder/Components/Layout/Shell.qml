import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

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

    SplitView {
        id: horizontalSplit
        orientation: Qt.Horizontal
        Layout.row: 2
        Layout.fillWidth: true
        Layout.fillHeight: true

        property int minHeight: 100

        Component.onCompleted: {
            implicitHeight = Math.max(layoutRoot.height * 0.9, minHeight)
            leftWrapper.implicitWidth = Math.max(width * 0.25, 100)
            centerWrapper.implicitWidth = Math.max(width * 0.75, 200)
        }

        onWidthChanged: {
            leftWrapper.implicitWidth = Math.max(width * 0.25, 100)
            centerWrapper.implicitWidth = Math.max(width * 0.75, 200)
        }

        onHeightChanged: {
            if (height < minHeight)
                height = minHeight
        }

        Item {
            id: leftWrapper
            SidebarRouter { anchors.fill: parent }
        }

        Item {
            id: centerWrapper
            ContentRouter { anchors.fill: parent }
        }
    }

    StatusBar {
        id: statusBar
        Layout.row: 3
        Layout.preferredHeight: 36
        Layout.fillWidth: true
    }
}

