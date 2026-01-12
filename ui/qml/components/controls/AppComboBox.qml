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
        border.color: control.focused ? Theme.primary.lighter(140) : "#ddd"
        border.width: 1
    }

    popup: Popup {
        z: 999
        contentItem: ListView {
            model: control.model
            delegate: ItemDelegate {
                text: model.display
                onClicked: { control.currentIndex = index; control.close() }
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
