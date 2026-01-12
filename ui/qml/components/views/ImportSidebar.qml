import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

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
                text: qsTr("Import-Protokolle")
                font.pointSize: 14
                Layout.fillWidth: true
                color: Theme.textPrimary
            }

            AppButton {
                text: qsTr("Leeren")
                enabled: typeof uiImport !== 'undefined' && uiImport && uiImport.runs && uiImport.runs.count > 0
                implicitHeight: 32
                implicitWidth: 88
                // subtle secondary style: surface fill with primary text
                fillColor: Theme.surface
                textColor: Theme.textPrimary
                onClicked: if (typeof uiImport !== 'undefined' && uiImport && uiImport.runs) uiImport.runs.clear()
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "ImportRunsList.qml"
            onLoaded: {
                if (item) item.model = (typeof uiImport !== 'undefined' && uiImport) ? uiImport.runs : null
            }
        }
    }
}
