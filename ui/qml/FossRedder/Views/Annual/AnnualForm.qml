import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    readonly property AnnualController annualController: AppContext.annualController
    readonly property StateFacade session: AppContext.session
    Accessible.ignored: AppContext.isDebugBuild
    anchors.fill: parent

    function createAnnual() {
        if (!annualController) return
        try {
            var year = parseInt(yearField.text)
            if (isNaN(year)) return
            if (typeof annualController.addAnnual === 'function') {
                var id = annualController.addAnnual(year)
                if (id && id.length > 0) session.selectedAnnualId = id
            }
        } catch(e) {
        }
        if (stackView) stackView.pop()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.spacingMedium

        Label { text: qsTr("New Year"); font.pointSize: Theme.fontSizeTitle + Theme.margins }

        Controls.TextField { id: yearField; placeholderText: qsTr("Year (e.g. 2025)") }

        RowLayout { Layout.fillWidth: true
            Controls.Button { text: qsTr("Cancel"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Create")
                enabled: yearField.text.length > 0
                onClicked: createAnnual()
            }
        }
    }
}
