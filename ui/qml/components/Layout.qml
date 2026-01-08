import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

GridLayout {
    id: layoutRoot
    anchors.fill: parent
    columns: 1
    columnSpacing: 0
    rowSpacing: 0

    Loader {
        id: toolbarLoader
        source: "Toolbar.qml"
        asynchronous: false
        Layout.row: 0
        Layout.preferredHeight: 64
        Layout.fillWidth: true
    }

    SplitView {
        id: verticalSplit
        orientation: Qt.Vertical
        Layout.row: 1
        Layout.fillWidth: true
        Layout.fillHeight: true

        property int minTopHeight: 100
        property int minBottomHeight: 36

        Component.onCompleted: {
            horizontalSplit.implicitHeight = Math.max(verticalSplit.height * 0.9, minTopHeight)

            leftWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.25, 100)
            centerWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.75, 200)
        }

        onHeightChanged: {
            horizontalSplit.implicitHeight = Math.max(verticalSplit.height * 0.9, minTopHeight)
        }

        SplitView {
            id: horizontalSplit
            orientation: Qt.Horizontal
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: leftWrapper

                Loader {
                    anchors.fill: parent
                    source: "SidebarLeft.qml"
                    asynchronous: false
                }
            }

            Item {
                id: centerWrapper

                Loader {
                    anchors.fill: parent
                    source: "ContentArea.qml"
                    asynchronous: false
                }
            }

            onWidthChanged: {
                leftWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.25, 100)
                centerWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.75, 200)
            }

            onHeightChanged: {
                if (height < verticalSplit.minTopHeight) height = verticalSplit.minTopHeight
            }
        }
    }

    Loader {
        id: statusLoader
        source: "StatusBar.qml"
        asynchronous: false
        Layout.row: 2
        Layout.preferredHeight: 36
        Layout.fillWidth: true
    }
}
