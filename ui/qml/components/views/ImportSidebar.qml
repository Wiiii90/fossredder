import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Import Logs"); font.pointSize: 14; Layout.fillWidth: true }
            Button {
                text: qsTr("Clear")
                enabled: typeof uiImport !== 'undefined' && uiImport && uiImport.runs && uiImport.runs.count > 0
                onClicked: if (typeof uiImport !== 'undefined' && uiImport && uiImport.runs) uiImport.runs.clear()
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "ImportRunsList.qml"
            onLoaded: {
                item.model = (typeof uiImport !== 'undefined' && uiImport) ? uiImport.runs : null
            }
        }
    }
}
