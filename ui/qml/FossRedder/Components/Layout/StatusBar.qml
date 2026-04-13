import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: bar
    required property var appContext
    required property var theme
    readonly property string readyStatusText: qsTr("Ready")

    Rectangle {
        anchors.fill: parent
        color: bar.theme.statusBarBackground
        border.width: bar.theme.borderWidthThin
        border.color: bar.theme.statusBarBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: bar.theme.spacingMedium

            Label {
                id: lblStatus
                text: (bar.appContext && bar.appContext.status && bar.appContext.status.text && bar.appContext.status.text.length > 0)
                      ? bar.appContext.status.text
                      : bar.readyStatusText
                color: bar.theme.textPrimary
                elide: Label.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
