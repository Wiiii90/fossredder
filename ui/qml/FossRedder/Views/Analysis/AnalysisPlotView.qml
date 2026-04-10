import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Components 1.0 as Components
import "../../Constants/Analysis.js" as Analysis

Item {
    id: root
    readonly property StateFacade session: AppContext.session
    Accessible.ignored: AppContext.isDebugBuild
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []

    Timer { id: rebuildRetry; interval: 80; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { rebuild() } catch(e) {} } }

    function currentPlotType() {
        try {
            return Analysis.plotType(session ? session.lastAnalysisResult : null)
        } catch (e) {
            return ""
        }
    }

    function colorForKey(k) {
        return Analysis.colorForKey(k, Theme.analysis.palette, Theme.chartFallback)
    }

    function rebuild() {
        var newHist = []
        var newProps = []
        var newHistTotal = 0
        try {
            if (!session || !session.lastAnalysisResult || !session.lastAnalysisResult.table) {
                histLegendModel = newHist
                histLegendTotal = newHistTotal
                propListModel = newProps
                return
            }

            var contractMap = {}
            var propertyMap = {}
            var pieValues = []

            for (var i = 0; i < session.lastAnalysisResult.table.length; ++i) {
                var row = session.lastAnalysisResult.table[i]
                if (!row || row.length < 2) continue
                var parsed = false
                try {
                    var j = JSON.parse(row[1]); var bc = j.byContract || {}; var bp = j.byProperty || {}
                    for (var k in bc) contractMap[k] = (contractMap[k] || 0) + (parseFloat(bc[k]) || 0)
                    for (var k in bp) propertyMap[k] = (propertyMap[k] || 0) + (parseFloat(bp[k]) || 0)
                    parsed = true
                } catch(e) {}

                if (!parsed) {
                    var nv = NaN
                    var name = null
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

            for (var key in contractMap) {
                newHist.push({ name: key, value: contractMap[key] })
                newHistTotal += contractMap[key]
            }

            if (newHist.length === 0 && pieValues.length > 0) {
                newHist = pieValues.slice()
                newHistTotal = 0
                for (var pi = 0; pi < newHist.length; ++pi) newHistTotal += newHist[pi].value
            } else {
                newHist.sort(function(a, b) { return b.value - a.value })
            }

            for (var p in propertyMap) newProps.push({ name: p, value: propertyMap[p] })
            newProps.sort(function(a, b) { return b.value - a.value })

            histLegendModel = JSON.parse(JSON.stringify(newHist))
            histLegendTotal = newHistTotal
            propListModel = JSON.parse(JSON.stringify(newProps))

            try {
                try { if (pie) pie.legendFilter = session ? (session._legendFilter ? session._legendFilter : []) : [] } catch(e) {}
                try { if (hist) hist.legendFilter = session ? (session._legendFilter ? session._legendFilter : []) : [] } catch(e) {}
                try { if (session && session.propertyName) { if (pie) pie.propertyNameForId = function(id) { try { return session.propertyName(id) } catch(e) { return id } }; if (hist) hist.propertyNameForId = function(id) { try { return session.propertyName(id) } catch(e) { return id } } } else { if (pie) pie.propertyNameForId = null; if (hist) hist.propertyNameForId = null } } catch(e) {}

                var tbl = (session && session.lastAnalysisResult) ? session.lastAnalysisResult.table : []
                if (pie) pie.table = tbl
                if (hist) hist.table = tbl
                try { if (pie && pie.requestPaint) pie.requestPaint(); if (hist && hist.requestPaint) hist.requestPaint(); } catch(e) {}
            } catch(e) {}
        } catch(e) { }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            Item {
                id: pieArea
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.chartPlotPreferredHeight
                visible: (function() {
                    try {
                        return currentPlotType() === Analysis.chartTypes.pie
                    } catch (e) {
                        return false
                    }
                })()
            
                RowLayout { anchors.fill: parent; spacing: Theme.spacing
                    Column { id: pieLegendCol; Layout.preferredWidth: Theme.analysis.layout.splitControlsWidth; Layout.fillHeight: true; spacing: Theme.spacingSmall
                        Label { text: qsTr('Legend'); font.bold: false; color: Theme.chartText }
                        Repeater { model: (session && session.lastAnalysisResult) ? session.lastAnalysisResult.table : []
                            delegate: RowLayout { spacing: Theme.spacingSmall; height: 20
                                Rectangle { width: Theme.chartLegendMarkerSize; height: Theme.chartLegendMarkerSize; color: colorForKey(modelData && modelData.length>0 ? modelData[0] : index) }
                                Label { text: (modelData && modelData.length>0) ? modelData[0] : ''; horizontalAlignment: Text.AlignLeft; Layout.preferredWidth: 120 }
                                Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return ' ' + v.toFixed(2); })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: Theme.chartValueLabelWidth }
                                Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return (histLegendTotal>0) ? (' ' + ((parseFloat(modelData[1]) / histLegendTotal * 100).toFixed(1) + Analysis.text.percentSuffix)) : '' })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: Theme.chartPercentLabelWidth }
                            }
                        }
                    }

                    Components.Pie { id: pie; Layout.fillWidth: true; Layout.preferredHeight: Theme.chartPlotPreferredHeight }
                }
            }

            Components.Histogram {
                id: hist
                splitProgress: 0.0
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.chartPlotPreferredHeight
                visible: (function() {
                    try {
                        return currentPlotType() === Analysis.chartTypes.histogram
                    } catch (e) {
                        return false
                    }
                })()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium
            Item { Layout.fillWidth: true }
            RowLayout {
                spacing: Theme.spacingSmall
                visible: currentPlotType() === Analysis.chartTypes.histogram
                Label { text: qsTr('Split by property') }
                Switch { id: splitSwitch; onCheckedChanged: { hist.splitProgress = checked ? 1.0 : 0.0; try { if (hist.requestPaint) hist.requestPaint(); } catch(e) {} } }
            }
        }


        Flickable {
            id: histLegendFlick
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.chartLegendHeight
            clip: true
            contentWidth: histLegendFlow.implicitWidth
            contentHeight: histLegendFlow.implicitHeight
            visible: (histLegendModel && histLegendModel.length>0)

            Flow {
                id: histLegendFlow
                width: parent.width
                spacing: Theme.spacingMedium
                flow: Flow.LeftToRight

                Label { text: qsTr('Legend'); font.bold: false }
                Repeater {
                    id: histLegendRepeater
                    model: histLegendModel
                    delegate: RowLayout {
                        spacing: Theme.spacingSmall
                        Rectangle { width: Theme.chartLegendMarkerSize; height: Theme.chartLegendMarkerSize; color: colorForKey(modelData && modelData.name ? modelData.name : modelData) }
                        Label { text: (modelData && modelData.name) ? modelData.name : ''; font.pixelSize: Theme.fontSize }
                        Label { text: (modelData && modelData.value) ? (' ' + parseFloat(modelData.value).toFixed(2)) : (' ' + Analysis.text.defaultLegendValue); font.pixelSize: Theme.fontSize }
                        Label { text: (modelData && modelData.value && histLegendTotal>0) ? (' ' + ((parseFloat(modelData.value) / histLegendTotal * 100).toFixed(1) + Analysis.text.percentSuffix)) : '' ; font.pixelSize: Theme.fontSize }
                    }
                }
            }
        }


    }

    Connections { target: session
        function onLastAnalysisResultChanged() { try { rebuild() } catch(e) {} }
    }
}

