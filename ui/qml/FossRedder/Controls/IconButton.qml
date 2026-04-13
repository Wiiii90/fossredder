import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Effects
import FossRedder 1.0

Item {
    id: button
    implicitWidth: 100
    implicitHeight: 48

    width: button.parent && button.parent.width > 0 ? button.parent.width : implicitWidth
    height: button.parent && button.parent.height > 0 ? button.parent.height : implicitHeight

    property url svgSource: ""
    property string label: ""
    property bool active: false

    signal clicked()

    Rectangle { anchors.fill: parent; color: "transparent" }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        Item {
            width: Math.min(Math.round(button.implicitHeight * 0.65), Math.round(button.width * 0.9))
            height: width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            transformOrigin: Item.Center

            Item {
                id: iconContainer
                anchors.fill: parent
                property bool hovered: false
                property bool pressed: false
                scale: ((button.active || hovered) ? 1.08 : 1.0) * (pressed ? 0.96 : 1.0)

                Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutQuad } }

                MultiEffect {
                    anchors.fill: parent
                    source: iconSvg
                    colorization: button.active ? 0.20 : 0
                    colorizationColor: Theme.accent
                    visible: button.active
                    z: 2
                }

                Image {
                    id: iconSvg
                    anchors.centerIn: parent
                    source: button.svgSource
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: String(button.svgSource).length === 0 ? 0 : (button.active ? 0.96 : 1)
                    visible: status !== Image.Error
                    z: 1
                }
            }
        }

        Text {
            text: button.label
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Math.max(10, Math.round(button.height * 0.14))
            color: Theme.textPrimary
            Layout.alignment: Qt.AlignHCenter
            visible: button.label !== ""
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: 2
        onPressed: iconContainer.pressed = true
        onReleased: iconContainer.pressed = false
        onEntered: iconContainer.hovered = true
        onExited: iconContainer.hovered = false
        onClicked: button.clicked()
    }
}
