import QtQuick 2.15

Item {
    id: root
    required property var theme
    property string text: ""
    property string subtitle: ""
    property url iconSource: ""
    property bool selectable: true
    property bool selected: false
    property color selectedColor: root.theme.background
    property color backgroundColor: "transparent"
    property color borderColor: root.theme.borderSoft
    property real borderWidth: root.theme.borderWidthThin
    property real radius: 6
    property int padding: 8

    signal activated()

    implicitHeight: Math.max(40, contentRow.height + root.padding * 2)

    Rectangle {
        id: bg
        anchors.fill: parent
        color: root.selected ? root.selectedColor : root.backgroundColor
        radius: root.radius
        border.color: root.borderColor
        border.width: root.borderWidth

    }

    Row {
        id: contentRow
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 8

        Image {
            id: icon
            source: root.iconSource
            visible: root.iconSource && String(root.iconSource).length > 0
            width: visible ? 24 : 0
            height: visible ? 24 : 0
            fillMode: Image.PreserveAspectFit
        }

        Column {
            width: Math.max(0, root.width - (icon.visible ? icon.width + contentRow.spacing : 0) - root.padding * 2)
            spacing: 2

            Text {
                id: title
                text: root.text
                font.family: root.theme.fontFamily
                font.pointSize: Math.max(12, root.theme.fontSize - 2)
                font.bold: false
                color: root.theme.textPrimary
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                id: subtitle
                text: root.subtitle
                font.family: root.theme.fontFamily
                font.pointSize: Math.max(10, root.theme.fontSize - 4)
                color: root.theme.textMuted
                elide: Text.ElideRight
                visible: root.subtitle && root.subtitle.length > 0
                width: parent.width
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.selectable
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.activated()
    }
}
