import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    width: 240
    Column {
        anchors.fill: parent
        spacing: 8
        Label { text: qsTr("Annuals"); font.pointSize: 14 }
        ListView {
            model: uiData ? uiData.annuals : null
            delegate: Item {
                width: parent.width
                height: 36
                RowLayout { anchors.fill: parent; anchors.margins: 6; Label { text: model.year; Layout.fillWidth: true } Label { text: model.verificationState; opacity: 0.7 } }
                MouseArea { anchors.fill: parent; onClicked: if (uiData) uiData.selectedAnnualId = model.id }
            }
        }
    }
}
