import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                text: qsTr("Import logs")
                font.pointSize: Theme.fontSizeLarge
                Layout.fillWidth: true
                color: Theme.textPrimary
            }

            Controls.Button {
                text: qsTr("Clear")
                enabled: typeof importController !== 'undefined' && importController && importController.runs && importController.runs.count > 0
                fillColor: Theme.surface
                textColor: Theme.textPrimary
                onClicked: if (typeof importController !== 'undefined' && importController && importController.runs) importController.runs.clear()
            }
        }

        Components.RunLogList {
            id: runLogList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: (typeof importController !== 'undefined' && importController) ? importController.runs : null
        }
    }
}
