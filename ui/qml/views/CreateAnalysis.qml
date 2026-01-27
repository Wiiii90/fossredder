import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    anchors.fill: parent
    // stackView reference injected by caller (AnalysisView) so this page can pop itself
    property var stackView
    // opaque background so underlying preview is not visible while creating
    Rectangle { anchors.fill: parent; color: Theme.surface }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label { text: qsTr("Neue Analyse"); font.pointSize: 18 }

        AppTextField { id: nameField; placeholderText: qsTr("Name der Analyse") }

        // Choose strategy: Tab or Plot
        RowLayout { Layout.fillWidth: true; spacing: 8
            Label { text: qsTr("Strategie"); Layout.preferredWidth: 120 }
            ComboBox {
                id: strategyCombo
                Layout.fillWidth: true
                model: [ qsTr("Tab"), qsTr("Plot") ]
                currentIndex: 1
            }
        }

        // Plot subtype (only shown when Plot selected)
        RowLayout { Layout.fillWidth: true; visible: strategyCombo.currentIndex === 1; spacing: 8
            Label { text: qsTr("Plot-Typ"); Layout.preferredWidth: 120 }
            ComboBox { id: plotTypeCombo; Layout.fillWidth: true; model: [ qsTr("pie"), qsTr("histogram") ]; currentIndex: 0 }
        }

        GroupBox { title: qsTr("Filter"); Layout.fillWidth: true
            ColumnLayout { anchors.fill: parent; anchors.margins: 6; spacing: 6
                RowLayout { Layout.fillWidth: true; spacing: 8
                    Label { text: qsTr("Datum von"); Layout.preferredWidth: 120 }
                    AppTextField { id: dateFrom; placeholderText: qsTr("YYYY-MM-DD") }
                }
                RowLayout { Layout.fillWidth: true; spacing: 8
                    Label { text: qsTr("Datum bis"); Layout.preferredWidth: 120 }
                    AppTextField { id: dateTo; placeholderText: qsTr("YYYY-MM-DD") }
                }
                RowLayout { Layout.fillWidth: true; spacing: 8
                    Label { text: qsTr("Contract Type"); Layout.preferredWidth: 120 }
                    AppTextField { id: contractTypeField; placeholderText: qsTr("z.B. gas") }
                }
            }
        }

        GroupBox {
            title: qsTr("Vorkonfiguration")
            Layout.fillWidth: true
            ColumnLayout { anchors.fill: parent; anchors.margins: 6;
                AppTextField { id: cfgField; placeholderText: qsTr("Konfiguration (JSON)") }
            }
        }

        RowLayout { Layout.fillWidth: true
            AppButton { text: qsTr("Abbrechen"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Erstellen")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (!uiData || !uiDomain) return
                    var strategy = strategyCombo.currentIndex === 1 ? "plot" : "tab"
                    var config = ""
                    if (strategy === "plot") config = plotTypeCombo.currentText

                    var filterSpec = ""
                    if (dateFrom.text && dateFrom.text.length > 0) filterSpec += "date>=" + dateFrom.text
                    if (dateTo.text && dateTo.text.length > 0) {
                        if (filterSpec.length > 0) filterSpec += ";"
                        filterSpec += "date<=" + dateTo.text
                    }
                    if (contractTypeField.text && contractTypeField.text.length > 0) {
                        if (filterSpec.length > 0) filterSpec += ";"
                        filterSpec += "contract.type=" + contractTypeField.text
                    }

                    var id = uiDomain.addAnalysis(nameField.text, strategy, config, filterSpec)
                    if (id && id.length > 0) {
                        uiData.selectedAnalysisId = id
                        // compute initial result immediately and store in uiData
                        try {
                            var res = uiDomain.computeAnalysis(id, filterSpec)
                            uiData.lastAnalysisResult = res
                        } catch(e) {}
                        // replace create page with analysis detail
                        if (stackView) stackView.replace(Qt.resolvedUrl("qrc:/qml/views/AnalysisDetail.qml"), { stackView: stackView })
                        return
                    }
                    if (stackView) stackView.pop()
                }
            }
        }
    }
}
