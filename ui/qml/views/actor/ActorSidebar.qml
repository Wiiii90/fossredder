import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/common"

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // header removed per UX request

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: uiData ? uiData.actors : null
            spacing: Theme.spacingSmall

            delegate: ListRow {
                width: list.width
                text: model.name ? model.name : ""
                subtitle: model.type ? model.type : ""
                selected: uiData ? (model.id === uiData.selectedActorId) : false
                onActivated: { if (uiData) uiData.selectedActorId = model.id }
            }
        }
    }
}
