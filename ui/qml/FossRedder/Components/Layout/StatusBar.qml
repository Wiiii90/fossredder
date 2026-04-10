import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: bar
    readonly property string readyStatusText: qsTr("Ready")

    Rectangle {
        anchors.fill: parent
        color: Theme.statusBarBackground
        border.width: Theme.borderWidthThin
        border.color: Theme.statusBarBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium

            Label {
                id: lblStatus
                text: (typeof status !== 'undefined' && status && status.text && status.text.length > 0)
                      ? status.text
                      : bar.readyStatusText
                color: Theme.textPrimary
                elide: Label.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
