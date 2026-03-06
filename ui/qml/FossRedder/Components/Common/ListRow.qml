import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root
    property alias text: title.text
    property alias subtitle: subtitle.text
    property url iconSource: ""
    property bool selectable: true
    property bool selected: false
    property color selectedColor: Theme.background
    property color backgroundColor: "transparent"
    property color borderColor: Theme.borderSoft
    property real borderWidth: Theme.borderWidthThin
    property real radius: 6
    property int padding: 8

    signal activated()

    implicitHeight: Math.max(40, row.implicitHeight + padding)

    Rectangle {
        id: bg
        anchors.fill: parent
        color: root.selected ? root.selectedColor : root.backgroundColor
        radius: root.radius
        border.color: root.borderColor
        border.width: root.borderWidth

    }

        RowLayout {
        id: row
        anchors.fill: parent
        anchors.margins: padding
        spacing: 8

        Image {
            id: icon
            source: root.iconSource
            visible: root.iconSource && root.iconSource.length > 0
            width: visible ? 24 : 0
            height: visible ? 24 : 0
            fillMode: Image.PreserveAspectFit
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                id: title
                text: qsTr("")
                font.family: Theme.fontFamily
                font.pointSize: Math.max(12, Theme.fontSize - 2)
                font.bold: false
                color: Theme.textPrimary
                elide: Label.ElideRight
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
            }

            Label {
                id: subtitle
                text: qsTr("")
                font.family: Theme.fontFamily
                font.pointSize: Math.max(10, Theme.fontSize - 4)
                color: Theme.textMuted
                elide: Label.ElideLeft
                visible: root.subtitle && root.subtitle.length > 0
                Layout.preferredWidth: 120
                horizontalAlignment: Text.AlignRight
            }
        }

        Item { Layout.fillWidth: true }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.selectable
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.activated()
    }
}
