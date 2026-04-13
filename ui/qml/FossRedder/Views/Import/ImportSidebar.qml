import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var importController: root.appContext ? root.appContext.importController : null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                text: qsTr("Import logs")
                font.pointSize: root.theme.fontSizeLarge
                Layout.fillWidth: true
                color: root.theme.textPrimary
            }

            Controls.Button {
                text: qsTr("Clear")
                enabled: root.importController && root.importController.runs && root.importController.runs.count > 0
                fillColor: root.theme.surface
                textColor: root.theme.textPrimary
                onClicked: if (root.importController && root.importController.runs) root.importController.runs.clear()
            }
        }

        Components.RunLogList {
            id: runLogList
            theme: root.theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.importController ? root.importController.runs : null
        }
    }
}
