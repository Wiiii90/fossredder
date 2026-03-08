import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Components 1.0 as Components

Item {
    id: root
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []

    function hashString(s) {
        var h = 0
        if (!s) return 0
        for (var i = 0; i < s.length; ++i) { h = ((h << 5) - h) + s.charCodeAt(i); h |= 0 }
        return Math.abs(h)
    }

    Timer { id: rebuildRetry; interval: 80; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { rebuild() } catch(e) {} } }

    function detectPlotType() {
        try {
            if (!uiData || !uiData.lastAnalysisResult || !uiData.lastAnalysisResult.table) return ""
            var tbl = uiData.lastAnalysisResult.table
            if (!tbl || tbl.length === 0) return ""
            var sample = tbl[0]
            if (!sample || sample.length < 2) return ""
            var col = (sample.length > 1) ? sample[1] : null
            var col2 = (sample.length > 2) ? sample[2] : null
            try {
                var j = JSON.parse(col)
                if (j && (j.total !== undefined || j.byContract !== undefined || j.byProperty !== undefined)) return "histogram"
            } catch(e) {}
            var n = parseFloat(col)
            if (!isNaN(n)) return "pie"
            var n2 = parseFloat(col2)
            if (!isNaN(n2)) return "pie"
            return ""
        } catch(e) { return "" }
    }

    function colorForKey(k) {
        try {
            var palette = Theme.analysisPalette
            return palette[hashString(k) % palette.length]
        } catch(e) { return Theme.chartFallback }
    }

    function rebuild() {
        var newHist = []
        var newProps = []
        var newHistTotal = 0
        try {
            if (typeof uiData === 'undefined' || !uiData || !uiData.lastAnalysisResult || !uiData.lastAnalysisResult.table) {
                histLegendModel = newHist
                histLegendTotal = newHistTotal
                propListModel = newProps
                return
            }

            var contractMap = {}
            var propertyMap = {}
            var pieValues = []

            for (var i = 0; i < uiData.lastAnalysisResult.table.length; ++i) {
                var row = uiData.lastAnalysisResult.table[i]
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
                try { if (pie) pie.legendFilter = uiData ? (uiData._legendFilter ? uiData._legendFilter : []) : [] } catch(e) {}
                try { if (hist) hist.legendFilter = uiData ? (uiData._legendFilter ? uiData._legendFilter : []) : [] } catch(e) {}
                try { if (uiData && uiData.propertyName) { if (pie) pie.propertyNameForId = function(id) { try { return uiData.propertyName(id) } catch(e) { return id } }; if (hist) hist.propertyNameForId = function(id) { try { return uiData.propertyName(id) } catch(e) { return id } } } else { if (pie) pie.propertyNameForId = null; if (hist) hist.propertyNameForId = null } } catch(e) {}

                var tbl = (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []
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
                Layout.preferredHeight: 320
                visible: (function() {
                    var t = ""
                    try {
                        if (uiData && uiData.lastAnalysisResult && uiData.lastAnalysisResult.type) t = uiData.lastAnalysisResult.type
                        if (!t) { var d = detectPlotType(); if (d) t = d }
                        return t === 'pie'
                    } catch(e) { return false }
                })()
            
                RowLayout { anchors.fill: parent; spacing: Theme.spacing
                    Column { id: pieLegendCol; Layout.preferredWidth: 220; Layout.fillHeight: true; spacing: Theme.spacingSmall
                        Label { text: qsTr('Legend'); font.bold: false; color: Theme.chartText }
                        Repeater { model: (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []
                            delegate: RowLayout { spacing: Theme.spacingSmall; height: 20
                                Rectangle { width: 10; height: 10; color: colorForKey(modelData && modelData.length>0 ? modelData[0] : index) }
                                Label { text: (modelData && modelData.length>0) ? modelData[0] : ''; horizontalAlignment: Text.AlignLeft; Layout.preferredWidth: 120 }
                                Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return ' ' + v.toFixed(2); })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: 80 }
                                Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return (histLegendTotal>0) ? (' ' + ((parseFloat(modelData[1]) / histLegendTotal * 100).toFixed(1) + '%')) : '' })(); horizontalAlignment: Text.AlignRight; Layout.preferredWidth: 60 }
                            }
                        }
                    }

                    Components.Pie { id: pie; Layout.fillWidth: true; Layout.preferredHeight: 320 }
                }
            }

            Components.Histogram {
                id: hist
                splitProgress: 0.0
                Layout.fillWidth: true
                Layout.preferredHeight: 320
                visible: (function() {
                    var t = ""
                    try {
                        if (uiData && uiData.lastAnalysisResult && uiData.lastAnalysisResult.type) t = uiData.lastAnalysisResult.type
                        if (!t) { var d = detectPlotType(); if (d) t = d }
                        return t === 'histogram'
                    } catch(e) { return false }
                })()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium
            Item { Layout.fillWidth: true }
            RowLayout {
                spacing: Theme.spacingSmall
                visible: Boolean(uiData && uiData.lastAnalysisResult && (uiData.lastAnalysisResult.type === 'histogram' || (uiData.lastAnalysisResult.config && (function(){ try { var c = JSON.parse(uiData.lastAnalysisResult.config); return c.plotType === 'histogram' } catch(e){ return false } })())) )
                Label { text: qsTr('Split by property') }
                Switch { id: splitSwitch; onCheckedChanged: { hist.splitProgress = checked ? 1.0 : 0.0; try { if (hist.requestPaint) hist.requestPaint(); } catch(e) {} } }
            }
        }


        Flickable {
            id: histLegendFlick
            Layout.fillWidth: true
            Layout.preferredHeight: 120
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
                        Rectangle { width: 10; height: 10; color: colorForKey(modelData && modelData.name ? modelData.name : modelData) }
                        Label { text: (modelData && modelData.name) ? modelData.name : ''; font.pixelSize: 12 }
                        Label { text: (modelData && modelData.value) ? (' ' + parseFloat(modelData.value).toFixed(2)) : ' 0.00'; font.pixelSize: 12 }
                        Label { text: (modelData && modelData.value && histLegendTotal>0) ? (' ' + ((parseFloat(modelData.value) / histLegendTotal * 100).toFixed(1) + '%')) : '' ; font.pixelSize: 12 }
                    }
                }
            }
        }


    }

    Connections { target: (typeof uiData !== 'undefined') ? uiData : null
        function onLastAnalysisResultChanged() { try { rebuild() } catch(e) {} }
    }
}

