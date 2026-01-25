import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

GridLayout {
    id: layoutRoot
    anchors.fill: parent
    columns: 1
    columnSpacing: 0
    rowSpacing: 0

    // Toolbar
    Loader {
        id: toolbarLoader
        source: "qrc:/qml/components/layout/Toolbar.qml"
        asynchronous: false
        Layout.row: 0
        Layout.preferredHeight: 64
        Layout.fillWidth: true
    }

    // Main split
    SplitView {
        id: horizontalSplit
        orientation: Qt.Horizontal
        Layout.row: 1
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

        // Sidebar
        Item {
            id: leftWrapper
            Loader {
                source: "qrc:/qml/components/layout/SidebarRouter.qml"
                asynchronous: false
                anchors.fill: parent
            }
        }

        // Content
        Item {
            id: centerWrapper
            Loader {
                source: "qrc:/qml/components/layout/ContentRouter.qml"
                asynchronous: false
                anchors.fill: parent
            }
        }
    }

    // Status bar
    Loader {
        id: statusLoader
        source: "qrc:/qml/components/layout/StatusBar.qml"
        asynchronous: false
        Layout.row: 2
        Layout.preferredHeight: 36
        Layout.fillWidth: true
    }
}
