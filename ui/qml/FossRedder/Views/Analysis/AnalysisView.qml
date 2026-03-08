import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views

Item {
    id: root
    readonly property var plotTypeOptions: [
        { value: "pie", label: qsTr("Pie chart") },
        { value: "histogram", label: qsTr("Histogram") }
    ]
    readonly property var plotMeasureOptions: [
        { value: "totalAmount", label: qsTr("Total Amount") },
        { value: "count", label: qsTr("Count") },
        { value: "averageAmount", label: qsTr("Average Amount") }
    ]
    anchors.fill: parent

    StackView {
        id: stackView
        anchors.fill: parent

        Component { id: analysisDetailComp; Views.AnalysisDetail { stackView: stackView } }

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

                    GroupBox {
                        id: createBox
                        title: qsTr("Create new analysis")
                        Layout.fillWidth: true

                        ListModel { id: contractTypeList }
                        property var selectedProps: []
                        property var selectedContractTypes: []

                        function rebuildChoices() {
                            contractTypeList.clear()
                            if (analysisController) {
                                var ctypes = []
                                try { ctypes = analysisController.getContractTypes() } catch(e) { ctypes = [] }
                                if (ctypes && ctypes.length > 0) {
                                    for (var ci = 0; ci < ctypes.length; ++ci)
                                        contractTypeList.append({ text: ctypes[ci] })
                                }
                            }
                            if (contractTypeList.count === 0)
                                contractTypeList.append({ text: qsTr("(All)") })

                            selectedContractTypes = []
                            for (var i = 0; i < contractTypeList.count; ++i) {
                                var t = contractTypeList.get(i).text
                                if (t && t !== qsTr("(All)")) selectedContractTypes.push(t)
                            }

                            selectedProps = []
                            try {
                                if (uiData && uiData.properties) {
                                    for (var j = 0; j < uiData.properties.length; ++j) {
                                        var p = uiData.properties[j]
                                        if (p && p.id) selectedProps.push(p.id)
                                    }
                                }
                            } catch(e) { selectedProps = [] }
                        }

                        Timer {
                            id: initTimer
                            interval: 200
                            repeat: true
                            running: false
                            triggeredOnStart: false
                            onTriggered: {
                                if (uiData && uiData.contracts && uiData.properties) { createBox.rebuildChoices(); stop(); }
                            }
                        }

                        Component.onCompleted: {
                            createBox.rebuildChoices()
                            if (!(uiData && uiData.contracts && uiData.properties))
                                initTimer.start()
                        }


                        onVisibleChanged: {
                            if (visible) {
                                createBox.rebuildChoices()
                            }
                        }

                        Connections {
                            target: stackView
                            function onCurrentItemChanged() {
                                if (stackView.currentItem === pageRect) {
                                    try { nameField.text = qsTr("Auto Analysis") } catch(e) {}
                                    try { dateFrom.text = "" } catch(e) {}
                                    try { dateTo.text = "" } catch(e) {}
                                    try { plotTypeCombo.currentIndex = 0 } catch(e) {}
                                    try { plotMeasureCombo.currentIndex = 0 } catch(e) {}
                                    createBox.rebuildChoices()
                                }
                            }
                        }

                        ColumnLayout { anchors.fill: parent; anchors.margins: 6; spacing: 8
                            Controls.TextField { id: nameField; placeholderText: qsTr("Analysis name") }

                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Strategy"); Layout.preferredWidth: 120 }
                                ComboBox { id: strategyCombo; Layout.fillWidth: true; model: [ qsTr("Tab"), qsTr("Plot"), qsTr("Calc") ]; currentIndex: 1 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: 8; visible: strategyCombo.currentIndex === 1
                                Label { text: qsTr("Plot type"); Layout.preferredWidth: 120 }
                                ComboBox { id: plotTypeCombo; Layout.fillWidth: true; model: root.plotTypeOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Date from"); Layout.preferredWidth: 120 }
                                Controls.TextField { id: dateFrom; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Date to"); Layout.preferredWidth: 120 }
                                Controls.TextField { id: dateTo; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            ColumnLayout { Layout.fillWidth: true; spacing: 6
                                Label { text: qsTr("Contract Types (select)"); Layout.preferredWidth: 120 }
                                Flickable { Layout.fillWidth: true; contentHeight: contractTypeList.count * 28; clip: true; height: 120
                                    Column { width: parent.width
                                        Repeater { model: contractTypeList
                                            delegate: RowLayout { width: parent.width; spacing: 8
                                                CheckBox {
                                                    id: ctcb
                                                    checked: true
                                                    Component.onCompleted: {
                                                        if (createBox.selectedContractTypes.indexOf(model.text) === -1) createBox.selectedContractTypes.push(model.text)
                                                    }
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedContractTypes.indexOf(model.text) === -1)
                                                                createBox.selectedContractTypes.push(model.text)
                                                        } else {
                                                            var idx = createBox.selectedContractTypes.indexOf(model.text)
                                                            if (idx !== -1) createBox.selectedContractTypes.splice(idx,1)
                                                        }
                                                    }
                                                }
                                                Label { text: model.text; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }

                            ColumnLayout { Layout.fillWidth: true; spacing: 6
                                Label { text: qsTr("Properties (select)") }
                                Flickable { Layout.fillWidth: true; contentHeight: propList.implicitHeight; clip: true; height: 120
                                    Column { id: propList; width: parent.width
                                        Repeater { model: uiData ? uiData.properties : []
                                            delegate: RowLayout { width: parent.width; spacing: 8
                                                CheckBox {
                                                    id: cb
                                                    checked: true
                                                    Component.onCompleted: {
                                                        if (createBox.selectedProps.indexOf(model.id) === -1) createBox.selectedProps.push(model.id)
                                                    }
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedProps.indexOf(model.id) === -1)
                                                                createBox.selectedProps.push(model.id)
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

                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Label { text: qsTr("Plot Measure"); Layout.preferredWidth: 120 }
                                ComboBox { id: plotMeasureCombo; Layout.fillWidth: true; model: root.plotMeasureOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: 8
                                Controls.Button {
                                    text: qsTr("Create")
                                    enabled: nameField.text.length > 0
                                    onClicked: {
                                        if (!uiData || !analysisController) return
                                        var strategy = "tab"
                                        if (strategyCombo.currentIndex === 1) strategy = "plot"
                                        else if (strategyCombo.currentIndex === 2) strategy = "calc"

                                        var selectedPlotType = root.plotTypeOptions[Math.max(0, plotTypeCombo.currentIndex)].value
                                        var selectedPlotMeasure = root.plotMeasureOptions[Math.max(0, plotMeasureCombo.currentIndex)].value

                                        var filterSpec = ""
                                        if (dateFrom.text && dateFrom.text.length > 0) filterSpec += "date>=" + dateFrom.text
                                        if (dateTo.text && dateTo.text.length > 0) {
                                            if (filterSpec.length > 0) filterSpec += ";"
                                            filterSpec += "date<=" + dateTo.text
                                        }

                                        createBox.rebuildChoices()

                                        var cfg = { plotType: selectedPlotType, plotMeasure: selectedPlotMeasure, properties: createBox.selectedProps, contractTypes: createBox.selectedContractTypes }
                                        var id = analysisController.addAnalysis(nameField.text, strategy, JSON.stringify(cfg), filterSpec)
                                        if (id && id.length > 0) {
                                            uiData.selectedAnalysisId = id
                                            try { var res = analysisController.computeAnalysis(id, filterSpec); uiData.lastAnalysisResult = res } catch(e) {}
                                            stackView.push(analysisDetailComp)
                                        }
                                    }
                                }
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
