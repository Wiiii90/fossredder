import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall
        ListView {
            id: annualList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.session ? root.session.annuals : null
            spacing: root.theme.spacingSmall
            delegate: Components.ListRow {
                id: annualRow
                required property var model
                theme: root.theme
                width: annualList.width
                text: annualRow.model.year ? annualRow.model.year : ""
                subtitle: annualRow.model.verificationState ? annualRow.model.verificationState : ""
                selected: root.session ? (annualRow.model.id === root.session.selectedAnnualId) : false
                onActivated: { if (root.session) root.session.selectedAnnualId = annualRow.model.id }
            }
        }
    }
}

