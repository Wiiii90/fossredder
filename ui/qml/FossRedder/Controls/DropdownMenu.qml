/**
 * @file ui/qml/FossRedder/Controls/DropdownMenu.qml
 * @brief Provides the DropdownMenu component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
pragma ComponentBehavior: Bound

ComboBox {
    id: control
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.controlHeight
    implicitHeight: Theme.controlHeight
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    leftPadding: Theme.controlPaddingHorizontal
    rightPadding: indicator.width + Theme.controlPaddingHorizontal
    topPadding: Theme.controlPaddingVertical
    bottomPadding: Theme.controlPaddingVertical

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: control.activeFocus ? Theme.primary.lighter(140) : Theme.borderMedium
        border.width: Theme.borderWidthThin
        implicitHeight: Theme.controlHeight
        anchors.fill: parent
        Behavior on border.color { ColorAnimation { duration: 160 } }
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
            border.width: Theme.borderWidthThin
            border.color: Theme.borderMedium
        }

        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, 280)
            model: control.delegateModel
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: ItemDelegate {
                required property int index
                required property var model
                width: ListView.view ? ListView.view.width : control.width
                text: model && model.display !== undefined ? model.display : ""
                enabled: !model || model.available !== false
                opacity: enabled ? 1.0 : 0.5
                font.family: Theme.fontFamily
                font.pointSize: Theme.fontSize
                onClicked: {
                    if (!enabled) return
                    control.currentIndex = index
                    control.activated(index)
                    popup.close()
                }
            }
        }
    }

    indicator: Rectangle {
        width: 36
        height: parent ? parent.height : Theme.controlHeight
        color: "transparent"
        anchors.right: parent ? parent.right : undefined
        anchors.verticalCenter: parent ? parent.verticalCenter : undefined

        Text {
            anchors.centerIn: parent
            text: "▾"
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pointSize: Theme.fontSize
        }
    }
}
