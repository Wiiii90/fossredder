import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var analysisController: root.appContext ? root.appContext.analysisController : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    readonly property var plotTypeOptions: [
        { value: Constants.Analysis.chartTypes.pie, label: qsTr("Pie chart") },
        { value: Constants.Analysis.chartTypes.histogram, label: qsTr("Histogram") }
    ]
    readonly property var plotMeasureOptions: [
        { value: "totalAmount", label: qsTr("Total Amount") },
        { value: "count", label: qsTr("Count") },
        { value: "averageAmount", label: qsTr("Average Amount") }
    ]
    anchors.fill: parent

    StackView {
        id: analysisStack
        anchors.fill: parent

        Component { id: analysisDetailComp; Views.AnalysisDetail { stackView: analysisStack; appContext: root.appContext; theme: root.theme } }

        pushEnter: Transition { NumberAnimation { duration: 0 } }
        pushExit: Transition { NumberAnimation { duration: 0 } }
        popEnter: Transition { NumberAnimation { duration: 0 } }
        popExit: Transition { NumberAnimation { duration: 0 } }

        initialItem: Component {
            Rectangle {
                id: pageRect
                width: analysisStack ? analysisStack.width : root.theme.analysis.layout.defaultWidth
                height: analysisStack ? analysisStack.height : root.theme.analysis.layout.defaultHeight

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: root.theme.pageMargin
                    spacing: root.theme.settings.spacing

                    GroupBox {
                        id: createBox
                        title: qsTr("Create new analysis")
                        Layout.fillWidth: true

                        ListModel { id: contractTypeList }
                        property var selectedProps: []
                        property var selectedContractTypes: []

                        function rebuildChoices() {
                            contractTypeList.clear()
                            if (root.analysisController) {
                                let ctypes = []
                                try { ctypes = root.analysisController.contractTypes() } catch(e) { ctypes = [] }
                                if (ctypes && ctypes.length > 0) {
                                    for (let ci = 0; ci < ctypes.length; ++ci)
                                        contractTypeList.append({ text: ctypes[ci] })
                                }
                            }
                            if (contractTypeList.count === 0)
                                contractTypeList.append({ text: qsTr("(All)") })

                            selectedContractTypes = []
                            for (let i = 0; i < contractTypeList.count; ++i) {
                                const t = contractTypeList.get(i).text
                                if (t && t !== qsTr("(All)")) selectedContractTypes.push(t)
                            }

                            selectedProps = []
                            try {
                                if (root.session) {
                                    const properties = root.session.propertyRows()
                                    for (let j = 0; j < properties.length; ++j) {
                                        const p = properties[j]
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
                                if (root.session) { createBox.rebuildChoices(); stop(); }
                            }
                        }

                        Component.onCompleted: {
                            createBox.rebuildChoices()
                            if (!root.session)
                                initTimer.start()
                        }


                        onVisibleChanged: {
                            if (visible) {
                                createBox.rebuildChoices()
                            }
                        }

                        Connections {
                            target: analysisStack
                            function onCurrentItemChanged() {
                                if (analysisStack.currentItem === pageRect) {
                                    try { nameField.text = qsTr("Auto Analysis") } catch(e) {}
                                    try { dateFrom.text = "" } catch(e) {}
                                    try { dateTo.text = "" } catch(e) {}
                                    try { plotTypeCombo.currentIndex = 0 } catch(e) {}
                                    try { plotMeasureCombo.currentIndex = 0 } catch(e) {}
                                    createBox.rebuildChoices()
                                }
                            }
                        }

                        ColumnLayout { anchors.fill: parent; anchors.margins: root.theme.chartPanelMargin; spacing: root.theme.settings.spacing
                            Controls.TextField { id: nameField; placeholderText: qsTr("Analysis name") }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing
                                Label { text: qsTr("Strategy"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.ComboBox { id: strategyCombo; Layout.fillWidth: true; model: [ qsTr("Tab"), qsTr("Plot"), qsTr("Calc") ]; currentIndex: 1 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing; visible: strategyCombo.currentIndex === 1
                                Label { text: qsTr("Plot type"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.ComboBox { id: plotTypeCombo; Layout.fillWidth: true; model: root.plotTypeOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing
                                Label { text: qsTr("Date from"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.TextField { id: dateFrom; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing
                                Label { text: qsTr("Date to"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.TextField { id: dateTo; placeholderText: qsTr("YYYY-MM-DD") }
                            }

                            ColumnLayout { Layout.fillWidth: true; spacing: root.theme.spacingSmall
                                Label { text: qsTr("Contract Types (select)"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Flickable { Layout.fillWidth: true; contentHeight: contractTypeList.count * 28; clip: true; Layout.preferredHeight: root.theme.chartLegendHeight
                                    Column { width: parent.width
                                        Repeater { model: contractTypeList
                                            delegate: RowLayout { id: contractTypeRow; required property var model; width: createBox.width; spacing: root.theme.settings.spacing
                                                Controls.CheckBox {
                                                    id: ctcb
                                                    checked: true
                                                    Component.onCompleted: {
                                                        if (createBox.selectedContractTypes.indexOf(contractTypeRow.model.text) === -1) createBox.selectedContractTypes.push(contractTypeRow.model.text)
                                                    }
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedContractTypes.indexOf(contractTypeRow.model.text) === -1)
                                                                createBox.selectedContractTypes.push(contractTypeRow.model.text)
                                                        } else {
                                                            const idx = createBox.selectedContractTypes.indexOf(contractTypeRow.model.text)
                                                            if (idx !== -1) createBox.selectedContractTypes.splice(idx,1)
                                                        }
                                                    }
                                                }
                                                Label { text: contractTypeRow.model.text; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }

                            ColumnLayout { Layout.fillWidth: true; spacing: root.theme.spacingSmall
                                Label { text: qsTr("Properties (select)") }
                                Flickable { Layout.fillWidth: true; contentHeight: propList.implicitHeight; clip: true; Layout.preferredHeight: root.theme.chartLegendHeight
                                    Column { id: propList; width: parent.width
                                        Repeater { model: root.session ? root.session.propertyRows() : []
                                            delegate: RowLayout { id: createPropRow; required property var modelData; width: createBox.width; spacing: root.theme.settings.spacing
                                                Controls.CheckBox {
                                                    id: cb
                                                    checked: true
                                                    Component.onCompleted: {
                                                        if (createBox.selectedProps.indexOf(createPropRow.modelData.id) === -1) createBox.selectedProps.push(createPropRow.modelData.id)
                                                    }
                                                    onCheckedChanged: {
                                                        if (checked) {
                                                            if (createBox.selectedProps.indexOf(createPropRow.modelData.id) === -1)
                                                                createBox.selectedProps.push(createPropRow.modelData.id)
                                                        } else {
                                                            const idx = createBox.selectedProps.indexOf(createPropRow.modelData.id)
                                                            if (idx !== -1) createBox.selectedProps.splice(idx,1)
                                                        }
                                                    }
                                                }
                                                Label { text: createPropRow.modelData.name; Layout.fillWidth: true }
                                            }
                                        }
                                    }
                                }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing
                                Label { text: qsTr("Plot Measure"); Layout.preferredWidth: root.theme.formLabelWidth }
                                Controls.ComboBox { id: plotMeasureCombo; Layout.fillWidth: true; model: root.plotMeasureOptions; textRole: "label"; currentIndex: 0 }
                            }

                            RowLayout { Layout.fillWidth: true; spacing: root.theme.settings.spacing
                                Controls.Button {
                                    text: qsTr("Create")
                                    enabled: nameField.text.length > 0
                                    onClicked: {
                                        if (!root.session || !root.analysisController) return
                                        const strategyType = strategyCombo.currentIndex === 0 ? "tab" : (strategyCombo.currentIndex === 1 ? "plot" : "calc")
                                        const selectedPlotType = root.plotTypeOptions[Math.max(0, plotTypeCombo.currentIndex)].value
                                        const selectedPlotMeasure = root.plotMeasureOptions[Math.max(0, plotMeasureCombo.currentIndex)].value
                                        const configJson = root.analysisController.analysisConfigJson(strategyType, selectedPlotType, selectedPlotMeasure, createBox.selectedProps, createBox.selectedContractTypes, 0.0)
                                        const filterSpec = root.analysisController.analysisFilterSpec(dateFrom.text, dateTo.text)
                                        const analysisId = root.analysisController.createAnalysis(nameField.text, strategyType, configJson, filterSpec)
                                        if (analysisId && analysisId.length > 0) {
                                            root.session.selectedAnalysisId = analysisId
                                            const result = root.analysisController.computeAnalysis(analysisId, filterSpec)
                                            if (result && Object.keys(result).length > 0) root.session.lastAnalysisResult = result
                                            Qt.callLater(function() { analysisStack.push(analysisDetailComp) })
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
