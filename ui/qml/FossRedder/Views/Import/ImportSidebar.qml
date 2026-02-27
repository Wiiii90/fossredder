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
        anchors.margins: 8
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
                enabled: typeof uiImport !== 'undefined' && uiImport && uiImport.runs && uiImport.runs.count > 0
                implicitHeight: 32
                implicitWidth: 88
                fillColor: Theme.surface
                textColor: Theme.textPrimary
                onClicked: if (typeof uiImport !== 'undefined' && uiImport && uiImport.runs) uiImport.runs.clear()
            }
        }

        Components.RunLogList {
            id: runLogList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: (typeof uiImport !== 'undefined' && uiImport) ? uiImport.runs : null
        }
    }
}
