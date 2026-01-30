import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQuick.Effects

Item {
    id: button
    implicitWidth: 100
    implicitHeight: 48

    width: parent && parent.width > 0 ? parent.width : implicitWidth
    height: parent && parent.height > 0 ? parent.height : implicitHeight

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
            id: iconWrap
            width: Math.min(Math.round(button.implicitHeight * 0.65), Math.round(button.width * 0.9))
            height: width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            transformOrigin: Item.Center

            Item {
                id: iconContainer
                anchors.fill: parent
                property real hoverMultiplier: 1.08
                property bool hovered: false
                property bool pressed: false
                scale: ((active || hovered) ? hoverMultiplier : 1.0) * (pressed ? 0.96 : 1.0)

                Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutQuad } }

                MultiEffect {
                    id: iconGlow
                    anchors.fill: parent
                    source: iconSvg
                    colorization: active ? 0.20 : 0
                    colorizationColor: (typeof Theme !== 'undefined') ? Theme.accent : "#FFB74D"
                    brightness: active ? 0.01 : 0
                    saturation: active ? 0.01 : 0
                    visible: active
                    z: 2
                }

                Image {
                    id: iconSvg
                    anchors.centerIn: parent
                    source: svgSource
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: svgSource === "" ? 0 : (active ? 0.96 : 1)
                    visible: status !== Image.Error
                    z: 1
                }
            }
        }

        Text {
            id: labelText
            text: label
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Math.max(10, Math.round(height * 0.14))
            color: typeof Theme !== 'undefined' ? Theme.textPrimary : "#213547"
            Layout.alignment: Qt.AlignHCenter
            visible: label !== ""
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: 2
        onPressed: { iconContainer.pressed = true }
        onReleased: { iconContainer.pressed = false }
        onEntered: { iconContainer.hovered = true }
        onExited: { iconContainer.hovered = false }
        onClicked: button.clicked()
    }
}
