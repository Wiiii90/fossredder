import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    anchors.fill: parent
    StackView {
        id: stackView
        anchors.fill: parent
        // disable push/pop animations for instant transitions
        pushEnter: Transition { NumberAnimation { duration: 0 } }
        pushExit: Transition { NumberAnimation { duration: 0 } }
        popEnter: Transition { NumberAnimation { duration: 0 } }
        popExit: Transition { NumberAnimation { duration: 0 } }
        initialItem: Component {
            Rectangle {
                id: pageRect
                width: stackView ? stackView.width : 800
                height: stackView ? stackView.height : 600
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    // Inline create form — always visible on the main Analysis view
                    GroupBox {
                        id: createBox
                        title: qsTr("Neue Analyse erstellen")
                        Layout.fillWidth: true
                        ListModel { id: contractTypeList }
                        property var selectedProps: []
                        property var selectedContractTypes: []
                        // use model signals instead of trying to read .count into properties

                        function rebuildChoices() {
                            console.log("rebuildChoices: called; contractsModel=", uiData && uiData.contracts ? uiData.contracts : null)
                            contractTypeList.clear()
                            // try to get contract types from uiDomain (comprehensive and safe)
                            if (uiDomain) {
                                var ctypes = []
                                try { ctypes = uiDomain.getContractTypes() } catch(e) { ctypes = [] }
                                if (ctypes && ctypes.length > 0) {
                                    for (var ci=0; ci<ctypes.length; ++ci) contractTypeList.append({ text: ctypes[ci] })
                                    console.log("rebuildChoices: got contract types from uiDomain", ctypes)
                                }
                            }
                            // fallback: inspect uiData.contracts model if available
                            if (contractTypeList.count === 0) {
                                // ensure at least (All) is present
                                contractTypeList.append({ text: qsTr("(All)") })
                            }
                            // clear any prior property selections — user must explicitly select properties to filter by them
                            selectedProps = []
                        }

                        Timer { id: initTimer; interval: 200; repeat: true; running: false; triggeredOnStart: false
                            onTriggered: {
                                if (uiData && uiData.contracts && uiData.properties) { createBox.rebuildChoices(); stop(); }
                            }
                        }
                        Component.onCompleted: {
                            createBox.rebuildChoices()
                            if (!(uiData && uiData.contracts && uiData.properties)) initTimer.start()
                        }
                        // watch for updates on uiData models
                        Connections {
                            target: uiData
                            function onLastAnalysisResultChanged() { /* no-op here */ }
                            // rebuild when models change
                            function onActorsChanged() { createBox.rebuildChoices() }
                            function onPropertiesChanged() { createBox.rebuildChoices() }
                            function onContractsChanged() { createBox.rebuildChoices() }
                        }
                        // reset selections when the create box becomes visible again (e.g. after returning from detail)
                        onVisibleChanged: {
                            if (visible) {
                                createBox.selectedProps = []
                                createBox.selectedContractTypes = []
                            }
                        }
                        // also reset the create form whenever this page becomes the current item in the stack (e.g. after popping detail)
                        Connections { target: stackView; function onCurrentItemChanged() {
                                if (stackView.currentItem === pageRect) {
                                    // reset inputs
                                    try { nameField.text = "" } catch(e) {}
                                    try { dateFrom.text = "" } catch(e) {}
                                    try { dateTo.text = "" } catch(e) {}
                                    try { plotTypeCombo.currentIndex = 0 } catch(e) {}
                                    try { plotMeasureCombo.currentIndex = 0 } catch(e) {}
                                    createBox.selectedProps = []
                                    createBox.selectedContractTypes = []
                                    createBox.rebuildChoices()
                                }
                            } }
                        ColumnLayout { anchors.fill: parent; anchors.margins: 6; spacing: 8
                            AppTextField { id: nameField; placeholderText: qsTr("Name der Analyse") }
                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Strategie"); Layout.preferredWidth: 120 }
                                ComboBox { id: strategyCombo; Layout.fillWidth: true; model: [ qsTr("Tab"), qsTr("Plot") ]; currentIndex: 1 }
                            }
                            RowLayout { Layout.fillWidth: true; spacing: 8; visible: strategyCombo.currentIndex === 1
                                Label { text: qsTr("Plot-Typ"); Layout.preferredWidth: 120 }
                                ComboBox { id: plotTypeCombo; Layout.fillWidth: true; model: [ qsTr("pie"), qsTr("histogram") ]; currentIndex: 0 }
                            }
                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Datum von"); Layout.preferredWidth: 120 }
                                AppTextField { id: dateFrom; placeholderText: qsTr("YYYY-MM-DD"); text: "2025-01-01" }
                            }
                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Datum bis"); Layout.preferredWidth: 120 }
                                AppTextField { id: dateTo; placeholderText: qsTr("YYYY-MM-DD"); text: "2026-01-01" }
                            }
                            ColumnLayout { Layout.fillWidth: true; spacing: 6
                                Label { text: qsTr("Contract Types (select)"); Layout.preferredWidth: 120 }
                                Flickable { Layout.fillWidth: true; contentHeight: contractTypeList.count * 28; clip: true; height: 120
                                    Column { width: parent.width
                                        Repeater { model: contractTypeList
                                            delegate: RowLayout { width: parent.width; spacing: 8
                                                CheckBox { id: ctcb; checked: createBox.selectedContractTypes.indexOf(model.text) !== -1; onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedContractTypes.indexOf(model.text) === -1) createBox.selectedContractTypes.push(model.text)
                                                        } else {
                                                            var idx = createBox.selectedContractTypes.indexOf(model.text)
                                                            if (idx !== -1) createBox.selectedContractTypes.splice(idx,1)
                                                        }
                                                    } }
                                                Label { text: model.text; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }
                            // properties multi-select
                            ColumnLayout { Layout.fillWidth: true; spacing: 6
                                Label { text: qsTr("Properties (select)") }
                                Flickable { Layout.fillWidth: true; contentHeight: propList.implicitHeight; clip: true; height: 120
                                    Column { id: propList; width: parent.width
                                        Repeater { model: uiData ? uiData.properties : []
                                            delegate: RowLayout {
                                                width: parent.width
                                                spacing: 8
                                                CheckBox {
                                                    id: cb
                                                    checked: createBox.selectedProps.indexOf(model.id) !== -1
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedProps.indexOf(model.id) === -1) createBox.selectedProps.push(model.id)
                                                        } else {
                                                            var idx = createBox.selectedProps.indexOf(model.id)
                                                            if (idx !== -1) createBox.selectedProps.splice(idx,1)
                                                        }
                                                    }
                                                }
                                                Label { text: model.name; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }
                            // plot measure selection
                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Plot Measure"); Layout.preferredWidth: 120 }
                                ComboBox { id: plotMeasureCombo; Layout.fillWidth: true; model: [ qsTr("Total Amount"), qsTr("Count"), qsTr("Average Amount") ]; currentIndex: 0 }
                            }
                            RowLayout { Layout.fillWidth: true; spacing: 8
                                AppButton { text: qsTr("Erstellen"); enabled: nameField.text.length > 0; onClicked: {
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
                                        // do not encode contract type into filterSpec here; include selectedContractTypes in config JSON
                                        // ensure contract type list is up-to-date
                                        createBox.rebuildChoices()

                                        // include selectedProps, plotMeasure, plotType and selected contract types in the config (as JSON)
                                        var cfg = { plotType: plotTypeCombo.currentText, plotMeasure: plotMeasureCombo.currentText, properties: createBox.selectedProps, contractTypes: createBox.selectedContractTypes }
                                        var id = uiDomain.addAnalysis(nameField.text, strategy, JSON.stringify(cfg), filterSpec)
                                        if (id && id.length > 0) {
                                            uiData.selectedAnalysisId = id
                                            try {
                                                var res = uiDomain.computeAnalysis(id, filterSpec)
                                                uiData.lastAnalysisResult = res
                                            } catch(e) {}
                                            // navigate to analysis detail page
                                            stackView.push("qrc:/qml/views/AnalysisDetail.qml", { stackView: stackView })
                                        }
                                    } }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "transparent" }
                }
            }
        }
    }
}
