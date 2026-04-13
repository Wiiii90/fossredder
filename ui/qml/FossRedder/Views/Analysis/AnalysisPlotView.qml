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
    readonly property var session: root.appContext ? root.appContext.session : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []

    Timer { id: rebuildRetry; interval: 80; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { root.rebuild() } catch(e) {} } }

    function currentPlotType() {
        try {
            return Constants.Analysis.plotType(root.session ? root.session.lastAnalysisResult : null)
        } catch (e) {
            return ""
        }
    }

    function colorForKey(k) {
        return Constants.Analysis.colorForKey(k, root.theme.analysis.palette, root.theme.chartFallback)
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
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingMedium

            Item {
                id: pieArea
                Layout.fillWidth: true
                Layout.preferredHeight: root.theme.chartPlotPreferredHeight
                visible: (function() {
                    try {
                        return root.currentPlotType() === Constants.Analysis.chartTypes.pie
                    } catch (e) {
                        return false
                    }
                })()
            
                RowLayout { anchors.fill: parent; spacing: root.theme.spacing
                    Column { id: pieLegendCol; Layout.preferredWidth: root.theme.analysis.layout.splitControlsWidth; Layout.fillHeight: true; spacing: root.theme.spacingSmall
                        Label { text: qsTr('Legend'); font.bold: false; color: root.theme.chartText }
                        Repeater { model: (root.session && root.session.lastAnalysisResult) ? root.session.lastAnalysisResult.table : []
                            delegate: RowLayout { id: pieLegendRow; required property var modelData; required property int index; spacing: root.theme.spacingSmall; height: 20
                Rectangle { width: root.theme.chartLegendMarkerSize; height: root.theme.chartLegendMarkerSize; color: root.colorForKey(pieLegendRow.modelData && pieLegendRow.modelData.length>0 ? pieLegendRow.modelData[0] : String(pieLegendRow.index)) }
                                Label { text: (pieLegendRow.modelData && pieLegendRow.modelData.length>0) ? pieLegendRow.modelData[0] : ''; horizontalAlignment: Text.AlignLeft; Layout.preferredWidth: 120 }
                                Label { text: (function(){ const v = (pieLegendRow.modelData && pieLegendRow.modelData.length>1) ? parseFloat(pieLegendRow.modelData[1])||0 : 0; return ' ' + v.toFixed(2); })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: root.theme.chartValueLabelWidth }
                                Label { text: (function(){ const v = (pieLegendRow.modelData && pieLegendRow.modelData.length>1) ? parseFloat(pieLegendRow.modelData[1])||0 : 0; return (root.histLegendTotal>0) ? (' ' + ((parseFloat(pieLegendRow.modelData[1]) / root.histLegendTotal * 100).toFixed(1) + Constants.Analysis.text.percentSuffix)) : '' })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: root.theme.chartPercentLabelWidth }
                            }
                        }
                    }

                    Components.Pie { id: pie; theme: root.theme; Layout.fillWidth: true; Layout.preferredHeight: root.theme.chartPlotPreferredHeight }
                }
            }

            Components.Histogram {
                id: hist
                appContext: root.appContext
                theme: root.theme
                splitProgress: 0.0
                Layout.fillWidth: true
                Layout.preferredHeight: root.theme.chartPlotPreferredHeight
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
            spacing: root.theme.spacingMedium
            Item { Layout.fillWidth: true }
            RowLayout {
                spacing: root.theme.spacingSmall
                visible: root.currentPlotType() === Constants.Analysis.chartTypes.histogram
                Label { text: qsTr('Split by property') }
                Switch { id: splitSwitch; onCheckedChanged: { hist.splitProgress = checked ? 1.0 : 0.0; try { if (hist.requestPaint) hist.requestPaint(); } catch(e) {} } }
            }
        }


        Flickable {
            id: histLegendFlick
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.chartLegendHeight
            clip: true
            contentWidth: histLegendFlow.implicitWidth
            contentHeight: histLegendFlow.implicitHeight
            visible: (root.histLegendModel && root.histLegendModel.length>0)

            Flow {
                id: histLegendFlow
                width: parent.width
                spacing: root.theme.spacingMedium
                flow: Flow.LeftToRight

                Label { text: qsTr('Legend'); font.bold: false }
                Repeater {
                    id: histLegendRepeater
                    model: root.histLegendModel
                    delegate: RowLayout { id: histLegendRow; required property var modelData;
                        spacing: root.theme.spacingSmall
                        Rectangle { width: root.theme.chartLegendMarkerSize; height: root.theme.chartLegendMarkerSize; color: root.colorForKey(histLegendRow.modelData && histLegendRow.modelData.name ? histLegendRow.modelData.name : histLegendRow.modelData) }
                        Label { text: (histLegendRow.modelData && histLegendRow.modelData.name) ? histLegendRow.modelData.name : ''; font.pixelSize: root.theme.fontSize }
                    Label { text: (histLegendRow.modelData && histLegendRow.modelData.value) ? (' ' + parseFloat(histLegendRow.modelData.value).toFixed(2)) : (' ' + Constants.Analysis.text.defaultLegendValue); font.pixelSize: root.theme.fontSize }
                    Label { text: (histLegendRow.modelData && histLegendRow.modelData.value && root.histLegendTotal>0) ? (' ' + ((parseFloat(histLegendRow.modelData.value) / root.histLegendTotal * 100).toFixed(1) + Constants.Analysis.text.percentSuffix)) : '' ; font.pixelSize: root.theme.fontSize }
                    }
                }
            }
        }


    }

    Connections { target: root.session
        function onLastAnalysisResultChanged() { try { root.rebuild() } catch(e) {} }
    }
}

