import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/common"
import components.controls 1.0

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: Theme.spacingSmall


        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: uiData ? uiData.properties : null
            spacing: Theme.spacingSmall

            delegate: ListRow {
                width: list.width
                text: model.name ? model.name : ""
                subtitle: model.address ? model.address : ""
                selected: uiData ? (model.id === uiData.selectedPropertyId) : false
                onActivated: { if (uiData) uiData.selectedPropertyId = model.id }
            }
        }
    }
}
