import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label { text: qsTr("New Year"); font.pointSize: 18 }

        Controls.TextField { id: yearField; placeholderText: qsTr("Year (e.g. 2025)") }

        RowLayout { Layout.fillWidth: true
            Controls.Button { text: qsTr("Cancel"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Create")
                enabled: yearField.text.length > 0
                onClicked: {
                    if (typeof annualController === 'undefined' || !annualController) return
                    try {
                        var y = parseInt(yearField.text)
                        if (isNaN(y)) return
                        if (typeof annualController.addAnnual === 'function') {
                            var id = annualController.addAnnual(y)
                            if (id && id.length > 0) uiData.selectedAnnualId = id
                        } else {
                            var idx = uiData.annuals.rowCount ? uiData.annuals.rowCount() : -1
                        }
                    } catch(e) {}
                    if (stackView) stackView.pop()
                }
            }
        }
    }
}
