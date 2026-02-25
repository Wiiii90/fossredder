import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/common"

Item {
    id: root
    width: 240

    Column {
        anchors.fill: parent
        spacing: 8

        // header removed per UX request

        ListView {
            id: list
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 40
            model: uiData ? uiData.analyses : null
            spacing: Theme.spacingSmall

            delegate: ListRow {
                width: list.width
                text: model.name ? model.name : ""
                subtitle: model.type ? model.type : ""
                selected: uiData ? (model.id === uiData.selectedAnalysisId) : false
                onActivated: { if (uiData) uiData.selectedAnalysisId = model.id }
            }
        }
    }
}
