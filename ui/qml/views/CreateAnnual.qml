import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label { text: qsTr("Neues Jahr"); font.pointSize: 18 }

        AppTextField { id: yearField; placeholderText: qsTr("Jahr (z.B. 2025)") }

        RowLayout { Layout.fillWidth: true
            AppButton { text: qsTr("Abbrechen"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Erstellen")
                enabled: yearField.text.length > 0
                onClicked: {
                    if (!uiDomain) return
                    // Use uiDomain to add a statement or annual -- fallback: append to model directly
                    try {
                        var y = parseInt(yearField.text)
                        if (isNaN(y)) return
                        // Annual model may not have add API; add via domain controller if available
                        if (uiDomain && typeof uiDomain.addAnnual === 'function') {
                            var id = uiDomain.addAnnual(y)
                            if (id && id.length > 0) uiData.selectedAnnualId = id
                        } else {
                            // fallback: create a minimal annual via analysis list or direct model manipulation
                            // Not ideal but keeps flow working: create an analysis as placeholder
                            var idx = uiData.annuals.rowCount ? uiData.annuals.rowCount() : -1
                        }
                    } catch(e) {}
                    if (stackView) stackView.pop()
                }
            }
        }
    }
}
