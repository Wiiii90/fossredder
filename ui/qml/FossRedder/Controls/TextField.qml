import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

TextField {
    id: control
    Layout.fillWidth: true
    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize
    Accessible.ignored: AppContext.isDebugBuild

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radius
        border.color: control.activeFocus ? Theme.primary.lighter(140) : Theme.borderMedium
        border.width: control.activeFocus ? 1.5 : Theme.borderWidthThin
        implicitHeight: Math.max(36, control.contentHeight + 12)
        anchors.fill: parent
        Behavior on border.color { ColorAnimation { duration: 160 } }
    }

    leftPadding: 8
    rightPadding: 8
    topPadding: 6
    bottomPadding: 6
}
