import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root
    width: 240

    Column {
        anchors.fill: parent
        spacing: 8

        Label { text: qsTr("Analysen"); font.pointSize: 14 }

        ListView {
            id: list
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 40
            model: uiData ? uiData.analyses : null

            delegate: Item {
                width: list.width
                height: 40

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    Label { text: model.name; Layout.fillWidth: true }
                    Label { text: model.type; opacity: 0.7 }
                }

                MouseArea { anchors.fill: parent; onClicked: if (uiData) uiData.selectedAnalysisId = model.id }
            }
        }
    }
}
