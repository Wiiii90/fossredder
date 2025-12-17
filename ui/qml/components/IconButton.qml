import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: button
    implicitWidth: 100
    implicitHeight: 48

    width: (parent && parent.width && parent.width > 0) ? parent.width : implicitWidth
    height: (parent && parent.height && parent.height > 0) ? parent.height : implicitHeight

    property url svgSource: ""
    property string label: ""
    property color hoverColor: button ? button.palette.mid : "#dfeff6"
    property real iconScale: 1.0
    signal clicked()

    Rectangle { anchors.fill: parent; color: "transparent" }

    ColumnLayout { 
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        Item {
            id: iconContainer
            width: Math.min(Math.round(button.height * 0.6), Math.round(button.width * 0.8))
            height: width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            transformOrigin: Item.Center
            scale: iconScale

            Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutQuad } }

            Image {
                id: iconSvg
                anchors.centerIn: parent
                source: svgSource
                width: parent.width
                height: parent.height
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: svgSource === "" ? 0 : 1
                visible: !(status === Image.Error)
            }

            MouseArea {
                id: iconMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onEntered: { button.iconScale = 1.2 }
                onExited:  { button.iconScale = 1.0 }
                onClicked: clicked()
            }
        }

        Text {
            id: emojiPlaceholder
            visible: (typeof isDebugBuild !== 'undefined' && isDebugBuild) ? (iconSvg.status === Image.Error || iconSvg.opacity === 0) : false
            text: "⏹"
            font.pixelSize: Math.round(height * 0.45)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            id: labelText
            text: label
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Math.max(10, Math.round(height * 0.18))
            color: button ? button.palette.windowText : "#213547"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
