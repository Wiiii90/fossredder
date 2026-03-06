import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "../../Constants/Text.js" as Text

Item {
    id: bar

    Rectangle {
        anchors.fill: parent
        color: Theme.statusBarBackground
        border.color: Theme.statusBarBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium

            Label {
                id: lblStatus
                text: (typeof uiStatus !== 'undefined' && uiStatus && uiStatus.text && uiStatus.text.length > 0)
                      ? uiStatus.text
                      : Text.statusReady
                color: Theme.textPrimary
                elide: Label.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
