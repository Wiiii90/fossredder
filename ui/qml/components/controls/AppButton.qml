import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Button {
    id: control
    hoverEnabled: true

    // no menu alias to avoid cyclic alias issues; callers should open menus manually

    // allow customizing the fill and text color per-instance
    // default to Theme.primary (safer than calling lighter() on color strings)
    property color fillColor: Theme.primary
    property color textColor: Theme.surface

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    implicitWidth: contentItem.implicitWidth + 32
    implicitHeight: Math.max(36, contentItem.implicitHeight + 12)
    focusPolicy: Qt.StrongFocus

    // overall background container so we can layer shadow / bg / overlay
    background: Item {
        anchors.fill: parent

        // subtle shadow layer (simulated)
        Rectangle {
            id: shadow
            anchors.fill: parent
            // press -> low y, hover -> slightly closer to surface (lift), idle -> lower shadow
            y: control.pressed ? 1 : (control.hovered ? 2 : 4)
            radius: Theme.radius
            color: "#000000"
            // increase hover shadow a bit so lift is clearly visible but still subtle
            opacity: control.enabled ? (control.pressed ? 0.12 : (control.hovered ? 0.09 : 0.04)) : 0.03
            z: -2
            Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutQuad } }
            Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutQuad } }
        }

        // main button surface
        Rectangle {
            id: bg
            anchors.fill: parent
            radius: Theme.radius
            // use instance fillColor so individual buttons can be tinted
            color: control.fillColor
            // stronger but still subtle hover border so "anleuchten" is clearer
            border.width: control.focus ? 1.0 : (control.hovered ? 0.9 : 0)
            border.color: control.focus ? (Theme.accent ? Theme.accent : Theme.primary) : (control.hovered ? (Theme.accent ? Theme.accent : Theme.primary) : "transparent")
            // smoother and stronger scale on hover; press still slightly reduces
            scale: control.pressed ? 0.985 : (control.hovered ? 1.02 : 1.0)
            z: 0

            Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutQuad } }
            Behavior on color { ColorAnimation { duration: 180; easing.type: Easing.OutQuad } }
        }

        // hover "anleuchten" highlight: emphasize warm yellow/orange (less blue)
        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            z: 1
            // stronger but still soft glow; fades in/out smoothly
            opacity: control.enabled ? (control.hovered ? 0.22 : 0.0) : 0.0
            gradient: Gradient {
                // top: warm yellow (more yellow-focused)
                GradientStop { position: 0.0; color: Qt.rgba(1.00, 0.95, 0.25, 0.40) }
                // mid-top: orange tint
                GradientStop { position: 0.20; color: Qt.rgba(1.00, 0.70, 0.20, 0.24) }
                // mid: soft warm yellow blending down
                GradientStop { position: 0.55; color: Qt.rgba(1.00, 0.85, 0.40, 0.12) }
                // bottom: transparent
                GradientStop { position: 1.0; color: Qt.rgba(1.00, 0.85, 0.40, 0.00) }
            }
            Behavior on opacity { NumberAnimation { duration: 260; easing.type: Easing.OutQuad } }
        }

        // keep overlay only for pressed state to darken slightly; avoid any white overlay on hover
        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            color: "#000000"
            opacity: control.enabled ? (control.pressed ? 0.06 : 0.0) : 0.02
            z: 2
            Behavior on opacity { NumberAnimation { duration: 140; easing.type: Easing.OutQuad } }
        }

        // subtle top highlight for lighter appearance (adapts to Theme)
        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top }
            height: 1
            color: Theme.surface
            opacity: control.enabled ? 0.012 : 0.0
            z: 3
        }
    }

    contentItem: Text {
        id: label
        text: control.text
        anchors.centerIn: parent
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        elide: Text.ElideRight
    }

    // disabled appearance
    states: State {
        name: "disabled"
        when: !control.enabled
        PropertyChanges { target: label; opacity: 0.55 }
        PropertyChanges { target: bg; opacity: 0.7 }
    }

    transitions: Transition {
        NumberAnimation { properties: "opacity, scale"; duration: 220; easing.type: Easing.OutQuad }
    }

    Accessible.name: control.text
}
