import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/common"

Item {
    width: 240
    Column {
        anchors.fill: parent
        spacing: 8
        ListView {
            model: uiData ? uiData.annuals : null
            spacing: Theme.spacingSmall
            delegate: ListRow {
                width: parent.width
                text: model.year ? model.year : ""
                subtitle: model.verificationState ? model.verificationState : ""
                selected: uiData ? (model.id === uiData.selectedAnnualId) : false
                onActivated: { if (uiData) uiData.selectedAnnualId = model.id }
            }
        }
    }
}
