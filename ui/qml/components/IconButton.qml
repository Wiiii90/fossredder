import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: button
    implicitWidth: 100
    implicitHeight: 48
    property string icon: ""
    property string label: ""
    property color hoverColor: button ? button.palette.mid : "#dfeff6"
    signal clicked()

    property int innerPadding: 8

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
    }

    Column {
        id: content
        width: Math.max(1, button.width - button.innerPadding*2)
        anchors.centerIn: parent
        spacing: 4
        anchors.verticalCenterOffset: 0

        Text { id: iconText; text: button.icon; font.pixelSize: Math.round(button.height * 0.30); color: button ? button.palette.windowText : "#213547"; horizontalAlignment: Text.AlignHCenter; anchors.horizontalCenter: parent.horizontalCenter }
        Text { id: labelText; text: button.label; font.pixelSize: Math.round(button.height * 0.18); color: button ? button.palette.windowText : "#213547"; horizontalAlignment: Text.AlignHCenter; anchors.horizontalCenter: parent.horizontalCenter }
    }

    Rectangle {
        id: hoverRect
        width: content.width
        height: content.height + button.innerPadding
        anchors.centerIn: parent
        color: mouseArea.containsMouse ? button.hoverColor : "transparent"
        radius: 0
        z: -1
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: button.clicked()
        cursorShape: Qt.PointingHandCursor
    }
}
