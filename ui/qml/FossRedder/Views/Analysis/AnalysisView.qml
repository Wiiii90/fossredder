import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
import "../../Constants/Analysis.js" as Analysis

Item {
    id: root
    Accessible.ignored: typeof isDebugBuild !== 'undefined' && isDebugBuild
    readonly property var plotTypeOptions: [
        { value: Analysis.chartTypes.pie, label: qsTr("Pie chart") },
        { value: Analysis.chartTypes.histogram, label: qsTr("Histogram") }
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
                width: stackView ? stackView.width : Theme.analysis.layout.defaultWidth
                height: stackView ? stackView.height : Theme.analysis.layout.defaultHeight

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.pageMargin
                    spacing: Theme.settings.spacing

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
                                if (uiData) {
                                    var properties = uiData.propertyRows()
                                    for (var j = 0; j < properties.length; ++j) {
                                        var p = properties[j]
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
                                if (uiData) { createBox.rebuildChoices(); stop(); }
                            }
                        }

                        Component.onCompleted: {
                            createBox.rebuildChoices()
                            if (!uiData)
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

                        ColumnLayout { anchors.fill: parent; anchors.margins: Theme.chartPanelMargin; spacing: Theme.settings.spacing
                            Controls.TextField { id: nameField; placeholderText: qsTr("Analysis name") }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing
                                Label { text: qsTr("Strategy"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.ComboBox { id: strategyCombo; Layout.fillWidth: true; model: [ qsTr("Tab"), qsTr("Plot"), qsTr("Calc") ]; currentIndex: 1 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing; visible: strategyCombo.currentIndex === 1
                                Label { text: qsTr("Plot type"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.ComboBox { id: plotTypeCombo; Layout.fillWidth: true; model: root.plotTypeOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing
                                Label { text: qsTr("Date from"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.TextField { id: dateFrom; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing
                                Label { text: qsTr("Date to"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.TextField { id: dateTo; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            ColumnLayout { Layout.fillWidth: true; spacing: Theme.spacingSmall
                                Label { text: qsTr("Contract Types (select)"); Layout.preferredWidth: Theme.formLabelWidth }
                                Flickable { Layout.fillWidth: true; contentHeight: contractTypeList.count * 28; clip: true; height: Theme.chartLegendHeight
                                    Column { width: parent.width
                                        Repeater { model: contractTypeList
                                            delegate: RowLayout { width: parent.width; spacing: Theme.settings.spacing
                                                Controls.CheckBox {
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

                            ColumnLayout { Layout.fillWidth: true; spacing: Theme.spacingSmall
                                Label { text: qsTr("Properties (select)") }
                                Flickable { Layout.fillWidth: true; contentHeight: propList.implicitHeight; clip: true; height: Theme.chartLegendHeight
                                    Column { id: propList; width: parent.width
                                        Repeater { model: uiData ? uiData.propertyRows() : []
                                            delegate: RowLayout { width: parent.width; spacing: Theme.settings.spacing
                                                Controls.CheckBox {
                                                    id: cb
                                                    checked: true
                                                    Component.onCompleted: {
                                                        if (createBox.selectedProps.indexOf(modelData.id) === -1) createBox.selectedProps.push(modelData.id)
                                                    }
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedProps.indexOf(modelData.id) === -1)
                                                                createBox.selectedProps.push(modelData.id)
                                                        } else {
                                                            var idx = createBox.selectedProps.indexOf(modelData.id)
                                                            if (idx !== -1) createBox.selectedProps.splice(idx,1)
                                                        }
                                                    }
                                                }
                                                Label { text: modelData.name; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing
                                Label { text: qsTr("Plot Measure"); Layout.preferredWidth: Theme.formLabelWidth }
                                Controls.ComboBox { id: plotMeasureCombo; Layout.fillWidth: true; model: root.plotMeasureOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: Theme.settings.spacing
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

                                        var cfg = { plotType: selectedPlotType, plotMeasure: selectedPlotMeasure, properties: createBox.selectedProps, contractTypes: createBox.selectedContractTypes }
                                        var id = analysisController.addAnalysis(nameField.text, strategy, JSON.stringify(cfg), filterSpec)
                                        if (id && id.length > 0) {
                                            uiData.selectedAnalysisId = id
                                            try { var res = analysisController.computeAnalysis(id, filterSpec); uiData.lastAnalysisResult = res } catch(e) {}
                                             Qt.callLater(function() { stackView.push(analysisDetailComp) })
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
