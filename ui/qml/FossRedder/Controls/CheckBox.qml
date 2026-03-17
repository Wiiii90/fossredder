import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

CheckBox {
    id: control
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter
    Accessible.ignored: typeof isDebugBuild !== 'undefined' && isDebugBuild

    font.family: Theme.fontFamily
    font.pointSize: Theme.fontSize

    rightPadding: Theme.spacingSmall
}
