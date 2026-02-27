import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views

Item {
    anchors.fill: parent
    StackView {
        id: stackView
        anchors.fill: parent

        Component { id: annualFormComp; Views.AnnualForm { } }

        initialItem: Component {
            Item {
                anchors.fill: parent
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Label { text: qsTr("Annual"); font.pointSize: 18 }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Selected Annual:") }
                        Label { text: uiData && uiData.selectedAnnual ? uiData.selectedAnnual.name : qsTr("(none)") }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Controls.Button { text: qsTr("Neues Jahr"); onClicked: { stackView.push(annualFormComp) } }
                        Item { Layout.fillWidth: true }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "transparent" }
                }
            }
        }
    }
}
