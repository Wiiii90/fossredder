/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisPlotView.qml
 * @brief Provides the AnalysisPlotView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Components 1.0 as Components
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    property string exportStateJson: "{}"
    signal exportStateChanged(string exportStateJson)
    readonly property var session: root.appContext ? root.appContext.session : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []
    property bool applyingExportState: false

    Timer { id: rebuildRetry; interval: 80; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { root.rebuild() } catch(e) {} } }

    function currentPlotType() {
        try {
            return Constants.Analysis.plotType(root.session ? root.session.lastAnalysisResult : null)
        } catch (e) {
            return ""
        }
    }

    function parseExportState() {
        if (!root.exportStateJson || String(root.exportStateJson).length === 0)
            return ({})
        try {
            const parsed = JSON.parse(root.exportStateJson)
            return parsed && typeof parsed === "object" ? parsed : ({})
        } catch (e) {
            return ({})
        }
    }

    function persistExportState() {
        if (root.applyingExportState)
            return
        const state = {
            pieLegendFilter: pie && pie.legendFilter ? pie.legendFilter.slice() : [],
            histogramSplitByProperty: !!splitSwitch.checked
        }
        root.exportStateChanged(JSON.stringify(state))
    }

    function applyExportState() {
        const state = root.parseExportState()
        root.applyingExportState = true
        try {
            const pieLegendFilter = state.pieLegendFilter && state.pieLegendFilter.length !== undefined
                    ? state.pieLegendFilter.slice()
                    : []
            if (pie)
                pie.legendFilter = pieLegendFilter

            const splitByProperty = !!state.histogramSplitByProperty
            splitSwitch.checked = splitByProperty
            hist.splitProgress = splitByProperty ? 1.0 : 0.0
            try {
                if (pie && pie.requestPaint)
                    pie.requestPaint()
                if (hist && hist.requestPaint)
                    hist.requestPaint()
            } catch (e) {
            }
        } finally {
            root.applyingExportState = false
        }
    }

    function colorForKey(k) {
        return Constants.Analysis.colorForKey(k, root.theme.analysis.palette, root.theme.chartFallback)
    }

    function splitControlsWidth() {
        return root.theme.analysis && root.theme.analysis.layout && typeof root.theme.analysis.layout.splitControlsWidth === "number"
                ? root.theme.analysis.layout.splitControlsWidth
                : 180
    }

    function spacingSmall() {
        return typeof root.theme.spacingSmall === "number" ? root.theme.spacingSmall : 6
    }

    function spacingMedium() {
        return typeof root.theme.spacingMedium === "number" ? root.theme.spacingMedium : 10
    }

    function chartPlotMinimumHeight() {
        return typeof root.theme.chartPlotMinimumHeight === "number" ? root.theme.chartPlotMinimumHeight : 180
    }

    function chartPlotPreferredHeight() {
        return typeof root.theme.chartPlotPreferredHeight === "number" ? root.theme.chartPlotPreferredHeight : 260
    }

    function chartLegendHeight() {
        return typeof root.theme.chartLegendHeight === "number" ? root.theme.chartLegendHeight : 80
    }

    function chartLegendMarkerSize() {
        return typeof root.theme.chartLegendMarkerSize === "number" ? root.theme.chartLegendMarkerSize : 12
    }

    function chartValueLabelWidth() {
        return typeof root.theme.chartValueLabelWidth === "number" ? root.theme.chartValueLabelWidth : 70
    }

    function chartPercentLabelWidth() {
        return typeof root.theme.chartPercentLabelWidth === "number" ? root.theme.chartPercentLabelWidth : 58
    }

    function tablePropertyColumnWidth() {
        return root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.propertyColumnWidth === "number"
                ? root.theme.analysis.table.propertyColumnWidth
                : 180
    }

    function tableAmountColumnWidth() {
        return root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.amountColumnWidth === "number"
                ? root.theme.analysis.table.amountColumnWidth
                : 120
    }

    function tableTotalColumnWidth() {
        return root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.totalColumnWidth === "number"
                ? root.theme.analysis.table.totalColumnWidth
                : 140
    }

    function tableRowHeight() {
        return root.theme.analysis && root.theme.analysis.table && typeof root.theme.analysis.table.rowHeight === "number"
                ? root.theme.analysis.table.rowHeight
                : 30
    }

    function rebuild() {
        let newHist = []
        let newProps = []
        let newHistTotal = 0
        try {
            if (!root.session || !root.session.lastAnalysisResult || !root.session.lastAnalysisResult.table) {
                root.histLegendModel = newHist
                root.histLegendTotal = newHistTotal
                root.propListModel = newProps
                return
            }

            const contractMap = {}
            const propertyMap = {}
            const pieValues = []

            for (let i = 0; i < root.session.lastAnalysisResult.table.length; ++i) {
                const row = root.session.lastAnalysisResult.table[i]
                if (!row || row.length < 2) continue
                let parsed = false
                try {
                    const j = JSON.parse(row[1]); const bc = j.byContract || {}; const bp = j.byProperty || {}
                    for (const k in bc) contractMap[k] = (contractMap[k] || 0) + (parseFloat(bc[k]) || 0)
                    for (const k in bp) propertyMap[k] = (propertyMap[k] || 0) + (parseFloat(bp[k]) || 0)
                    parsed = true
                } catch(e) {}

                if (!parsed) {
                    let nv = NaN
                    let name = null
                    if (row.length > 2) {
                        nv = parseFloat(row[2])
                        name = row[1]
                    } else {
                        nv = parseFloat(row[1])
                        name = row[0]
                    }
                    if (!isNaN(nv)) pieValues.push({ name: name, value: nv })
                }
            }

            for (const key in contractMap) {
                newHist.push({ name: key, value: contractMap[key] })
                newHistTotal += contractMap[key]
            }

            if (newHist.length === 0 && pieValues.length > 0) {
                newHist = pieValues.slice()
                newHistTotal = 0
                for (let pi = 0; pi < newHist.length; ++pi) newHistTotal += newHist[pi].value
            } else {
                newHist.sort(function(a, b) { return b.value - a.value })
            }

            for (const p in propertyMap) newProps.push({ name: p, value: propertyMap[p] })
            newProps.sort(function(a, b) { return b.value - a.value })

            root.histLegendModel = JSON.parse(JSON.stringify(newHist))
            root.histLegendTotal = newHistTotal
            root.propListModel = JSON.parse(JSON.stringify(newProps))

            try {
                try { if (pie) pie.legendFilter = root.session ? (root.session._legendFilter ? root.session._legendFilter : []) : [] } catch(e) {}
                try { if (hist) hist.legendFilter = root.session ? (root.session._legendFilter ? root.session._legendFilter : []) : [] } catch(e) {}
                try { if (root.session && root.session.propertyName) { if (pie) pie.propertyNameForId = function(id) { try { return root.session.propertyName(id) } catch(e) { return id } }; if (hist) hist.propertyNameForId = function(id) { try { return root.session.propertyName(id) } catch(e) { return id } } } else { if (pie) pie.propertyNameForId = null; if (hist) hist.propertyNameForId = null } } catch(e) {}

                const tbl = (root.session && root.session.lastAnalysisResult) ? root.session.lastAnalysisResult.table : []
                if (pie) pie.table = tbl
                if (hist) hist.table = tbl
                try { if (pie && pie.requestPaint) pie.requestPaint(); if (hist && hist.requestPaint) hist.requestPaint(); } catch(e) {}
            } catch(e) {}
        } catch(e) { }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.spacingSmall()

        RowLayout {
            Layout.fillWidth: true
            spacing: root.spacingMedium()

            Item {
                id: pieArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: root.chartPlotMinimumHeight()
                Layout.preferredHeight: root.chartPlotPreferredHeight()
                visible: (function() {
                    try {
                        return root.currentPlotType() === Constants.Analysis.chartTypes.pie
                    } catch (e) {
                        return false
                    }
                })()
            
                RowLayout { Layout.fillWidth: true; Layout.fillHeight: true; spacing: root.spacingSmall()
                    Column { id: pieLegendCol; Layout.preferredWidth: root.splitControlsWidth(); Layout.fillHeight: true; spacing: root.spacingSmall()
                        Label { text: qsTr('Legend'); font.bold: false; color: root.theme.chartText || root.theme.textPrimary }
                        Repeater { model: (root.session && root.session.lastAnalysisResult) ? root.session.lastAnalysisResult.table : []
                            delegate: RowLayout { id: pieLegendRow; required property var modelData; required property int index; spacing: root.spacingSmall(); Layout.preferredHeight: 20
                Rectangle { Layout.preferredWidth: root.chartLegendMarkerSize(); Layout.preferredHeight: root.chartLegendMarkerSize(); color: root.colorForKey(pieLegendRow.modelData && pieLegendRow.modelData.length>0 ? pieLegendRow.modelData[0] : String(pieLegendRow.index)) }
                                Label { text: (pieLegendRow.modelData && pieLegendRow.modelData.length>0) ? pieLegendRow.modelData[0] : ''; horizontalAlignment: Text.AlignLeft; Layout.preferredWidth: 120 }
                                Label { text: (function(){ const v = (pieLegendRow.modelData && pieLegendRow.modelData.length>1) ? parseFloat(pieLegendRow.modelData[1])||0 : 0; return ' ' + v.toFixed(2); })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: root.chartValueLabelWidth() }
                                Label { text: (function(){ const v = (pieLegendRow.modelData && pieLegendRow.modelData.length>1) ? parseFloat(pieLegendRow.modelData[1])||0 : 0; return (root.histLegendTotal>0) ? (' ' + ((parseFloat(pieLegendRow.modelData[1]) / root.histLegendTotal * 100).toFixed(1) + Constants.Analysis.text.percentSuffix)) : '' })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: root.chartPercentLabelWidth() }
                            }
                        }
                    }

                    Components.Pie {
                        id: pie
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                Layout.minimumHeight: root.chartPlotMinimumHeight()
                        onLegendFilterChangedByUser: root.persistExportState()
                    }
                }
            }

            Components.Histogram {
                id: hist
                appContext: root.appContext
                theme: root.theme
                splitProgress: 0.0
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: root.chartPlotMinimumHeight()
                Layout.preferredHeight: root.chartPlotPreferredHeight()
                visible: (function() {
                    try {
                        return root.currentPlotType() === Constants.Analysis.chartTypes.histogram
                    } catch (e) {
                        return false
                    }
                })()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: root.spacingMedium()
            Item { Layout.fillWidth: true }
            RowLayout {
                spacing: root.spacingSmall()
                visible: root.currentPlotType() === Constants.Analysis.chartTypes.histogram
                Label { text: qsTr('Split by property'); color: root.theme.chartText || root.theme.textPrimary }
                Switch {
                    id: splitSwitch
                    objectName: "analysisHistogramSplitSwitch"
                    onCheckedChanged: {
                        hist.splitProgress = checked ? 1.0 : 0.0
                        try { if (hist.requestPaint) hist.requestPaint(); } catch(e) {}
                        root.persistExportState()
                    }
                }
            }
        }


        Flickable {
            id: histLegendFlick
            Layout.fillWidth: true
            Layout.preferredHeight: root.chartLegendHeight()
            clip: true
            contentWidth: histLegendFlow.implicitWidth
            contentHeight: histLegendFlow.implicitHeight
            visible: (root.histLegendModel && root.histLegendModel.length>0)

            Flow {
                id: histLegendFlow
                width: parent.width
                spacing: root.spacingMedium()
                flow: Flow.LeftToRight

                Label { text: qsTr('Legend'); font.bold: false }
                Repeater {
                    id: histLegendRepeater
                    model: root.histLegendModel
                    delegate: RowLayout { id: histLegendRow; required property var modelData;
                        spacing: root.spacingSmall()
                        Rectangle { Layout.preferredWidth: root.chartLegendMarkerSize(); Layout.preferredHeight: root.chartLegendMarkerSize(); color: root.colorForKey(histLegendRow.modelData && histLegendRow.modelData.name ? histLegendRow.modelData.name : histLegendRow.modelData) }
                        Label { text: (histLegendRow.modelData && histLegendRow.modelData.name) ? histLegendRow.modelData.name : ''; font.pixelSize: root.theme.fontSize }
                    Label { text: (histLegendRow.modelData && histLegendRow.modelData.value) ? (' ' + parseFloat(histLegendRow.modelData.value).toFixed(2)) : (' ' + Constants.Analysis.text.defaultLegendValue); font.pixelSize: root.theme.fontSize }
                    Label { text: (histLegendRow.modelData && histLegendRow.modelData.value && root.histLegendTotal>0) ? (' ' + ((parseFloat(histLegendRow.modelData.value) / root.histLegendTotal * 100).toFixed(1) + Constants.Analysis.text.percentSuffix)) : '' ; font.pixelSize: root.theme.fontSize }
                    }
                }
            }
        }


    }

    onExportStateJsonChanged: {
        root.applyExportState()
    }

    Connections { target: root.session
        function onLastAnalysisResultChanged() { try { root.rebuild() } catch(e) {} }
    }

    Component.onCompleted: {
        root.applyExportState()
    }
}

