import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

ComboBox {
    id: control
    Layout.fillWidth: true
    Layout.preferredHeight: 40
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    // harmonized background
    background: Rectangle {
        id: bg
        color: Theme.surface
        radius: Theme.radius
        border.color: control.focused ? Theme.primary.lighter(140) : Theme.border
        border.width: 1
    }

    popup: Popup {
        id: popup
        y: control.height
        width: control.width
        z: 999
        padding: 0
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: Theme.radius
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
        }

        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, 280)
            model: control.delegateModel
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: ItemDelegate {
                width: ListView.view ? ListView.view.width : control.width
                text: model.display
                font.family: Theme.fontFamily
                font.pointSize: Theme.fontSize
                onClicked: {
                    control.currentIndex = index
                    control.activated(index)
                    control.close()
                }
            }
        }
    }

    indicator: Rectangle {
        width: 36; height: parent ? parent.height : 40
        color: "transparent"
        anchors.right: parent ? parent.right : undefined
        anchors.verticalCenter: parent ? parent.verticalCenter : undefined

        Text { anchors.centerIn: parent; text: "▾"; color: Theme.textPrimary }
    }
}
