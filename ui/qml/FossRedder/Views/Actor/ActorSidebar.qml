import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Components 1.0 as Components

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall


        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: uiData ? uiData.actors : null
            spacing: Theme.spacingSmall

            delegate: Components.ListRow {
                width: list.width
                text: model.name ? model.name : ""
                subtitle: model.type ? model.type : ""
                selected: uiData ? (model.id === uiData.selectedActorId) : false
                onActivated: { if (uiData) uiData.selectedActorId = model.id }
            }
        }
    }
}

