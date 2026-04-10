import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Components 1.0 as Components

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: session ? session.annuals : null
            spacing: Theme.spacingSmall
            delegate: Components.ListRow {
                width: ListView.view ? ListView.view.width : parent.width
                text: model.year ? model.year : ""
                subtitle: model.verificationState ? model.verificationState : ""
                selected: session ? (model.id === session.selectedAnnualId) : false
                onActivated: { if (session) session.selectedAnnualId = model.id }
            }
        }
    }
}

