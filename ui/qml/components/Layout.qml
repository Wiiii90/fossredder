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
            horizontalSplit.implicitHeight = Math.max(verticalSplit.height * 0.8, minTopHeight)
            bottomWrapper.implicitHeight = Math.max(verticalSplit.height * 0.2, minBottomHeight)

            leftWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.2, 100)
            centerWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.6, 200)
            rightWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.2, 100)
        }

        onHeightChanged: {
            horizontalSplit.implicitHeight = Math.max(verticalSplit.height * 0.8, minTopHeight)
            bottomWrapper.implicitHeight = Math.max(verticalSplit.height * 0.2, minBottomHeight)
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

            Item {
                id: rightWrapper

                Loader {
                    anchors.fill: parent
                    source: "SidebarRight.qml"
                    asynchronous: false
                }
            }

            onWidthChanged: {
                leftWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.2, 100)
                centerWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.6, 200)
                rightWrapper.implicitWidth = Math.max(horizontalSplit.width * 0.2, 100)
            }

            onHeightChanged: {
                if (height < verticalSplit.minTopHeight) height = verticalSplit.minTopHeight
            }
        }

        Item {
            id: bottomWrapper

            Loader {
                anchors.fill: parent
                source: "BottomBar.qml"
                asynchronous: false
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
