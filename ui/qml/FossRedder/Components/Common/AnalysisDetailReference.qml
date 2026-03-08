import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import "../../Constants/Analysis.js" as Analysis

Item {
    id: page
    property var stackView
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []
    function colorForKey(k) {
        return Analysis.colorForKey(k, Theme.analysisPalette, Theme.chartFallback)
    }
    function rebuildHistLegend() {
        histLegendModel = []
        histLegendTotal = 0
        propListModel = []
        try {
            if (!uiData || !uiData.lastAnalysisResult) return
            if (!uiData.lastAnalysisResult.table) return
            var contractMap = {}
            var propertyMap = {}
            for (var i=0;i<uiData.lastAnalysisResult.table.length;i++) {
                var row = uiData.lastAnalysisResult.table[i]
                if (!row || row.length < 2) continue
                try {
                    var j = JSON.parse(row[1])
                    var bc = j.byContract || {}
                    var bp = j.byProperty || {}
                    for (var k in bc) {
                        var v = parseFloat(bc[k]) || 0
                        if (!contractMap[k]) contractMap[k] = 0
                        contractMap[k] += v
                    }
                    for (var pk in bp) {
                        var pv = parseFloat(bp[pk]) || 0
                        if (!propertyMap[pk]) propertyMap[pk] = 0
                        propertyMap[pk] += pv
                    }
                } catch(e) { }
            }
            for (var key in contractMap) { histLegendModel.push({ name: key, value: contractMap[key] }); histLegendTotal += contractMap[key] }
            histLegendModel.sort(function(a,b){ return b.value - a.value })
            for (var p in propertyMap) { propListModel.push({ name: p, value: propertyMap[p] }) }
            propListModel.sort(function(a,b){ return b.value - a.value })
        } catch(e) { }
    }
    
    width: stackView ? stackView.width : (parent ? parent.width : Analysis.layout.defaultWidth)
    height: stackView ? stackView.height : (parent ? parent.height : Analysis.layout.defaultHeight)
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.spacingMedium
        
        Connections { target: uiData; function onLastAnalysisResultChanged() { try { rebuildHistLegend(); } catch(e) {} } }

            RowLayout { Layout.fillWidth: true
            Controls.Button { text: qsTr("Back"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            Controls.Button { text: qsTr("Recompute"); onClicked: {
                if (!analysisController || !uiData || !uiData.selectedAnalysis) return
                var aid = uiData.selectedAnalysis.id
                var res = analysisController.computeAnalysis(aid, uiData.selectedAnalysis.filterSpec ? uiData.selectedAnalysis.filterSpec : "")
                try { uiData.lastAnalysisResult = res; rebuildHistLegend(); } catch(e) {}
            } }
        }

        GroupBox { title: qsTr("Preview"); Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout { anchors.fill: parent; anchors.margins: Theme.chartPanelMargin; spacing: Theme.chartPanelSpacing
                Timer { id: initialCanvasPaintTimer; interval: Analysis.layout.initialPaintIntervalMs; repeat: true; running: false; property int attempts: 0
                    onTriggered: {
                        try {
                            attempts++;
                            if (plotCanvas.width >= Analysis.layout.initialPaintReadyWidth && page.visible) {
                                plotCanvas.requestPaint(); rebuildHistLegend(); attempts = 0; stop();
                                return
                            }
                            if (attempts >= Analysis.layout.initialPaintMaxAttempts) { plotCanvas.requestPaint(); attempts = 0; stop(); }
                        } catch(e) { attempts = 0; stop(); }
                    }
                }

                ColumnLayout { Layout.fillWidth: true; spacing: Theme.chartPanelSpacing
                    Item { Layout.fillWidth: true; Layout.preferredHeight: Theme.chartPlotPreferredHeight; Layout.minimumHeight: Theme.chartPlotMinimumHeight
                        Canvas {
                            id: plotCanvas
                            property real splitProgress: 0.0
                            property int repaintAttempts: 0
                            Behavior on splitProgress { NumberAnimation { duration: Analysis.layout.splitAnimationDurationMs; easing.type: Easing.InOutQuad } }
                            anchors.fill: parent
                            focus: true
                            Timer { id: repaintTimer; interval: Analysis.layout.repaintDelayMs; repeat: false; onTriggered: plotCanvas.requestPaint() }
                            onSplitProgressChanged: { plotCanvas.requestPaint(); }
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.reset()
                                ctx.clearRect(0,0,width,height)
                                if (!page.visible || width < Analysis.layout.minRenderWidth) { if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start(); return }
                                if (!uiData || !uiData.lastAnalysisResult) return
                                plotCanvas.repaintAttempts = 0
                                var tbl = uiData.lastAnalysisResult.table
                                if (!tbl || tbl.length === 0) return
                                var type = Analysis.plotType(uiData.lastAnalysisResult)
                                if (width <= 0 || height <= 0) return
                                var cx = width/2; var cy = height/2
                                if (type === Analysis.chartTypes.pie) {
                                    var total = 0.0
                                    for (var i=0;i<tbl.length;i++) total += Math.abs(parseFloat(tbl[i][1]) || 0.0)
                                    var start = Analysis.render.pieStartAngle
                                    var radius = Math.min(width, height)/2 - Analysis.render.pieRadiusPadding
                                    if (radius <= 0) {
                                        var maxv = 0
                                        for (var i=0;i<tbl.length;i++) maxv = Math.max(maxv, Math.abs(parseFloat(tbl[i][1])||0))
                                        for (var i=0;i<tbl.length;i++) { var v = Math.abs(parseFloat(tbl[i][1])||0); var bw = Math.max(Analysis.render.compactBarMinWidth, Math.floor((width-Analysis.render.compactBarWidthPadding) * (v / Math.max(1, maxv)))); ctx.fillStyle = "hsl(" + ((i*360/tbl.length) % 360) + ",60%,60%)"; ctx.fillRect(Analysis.render.compactBarLeftPadding, i*Analysis.render.compactBarVerticalSpacing + Analysis.render.compactBarTopOffset, bw, Analysis.render.compactBarHeight) }
                                        return
                                    }
                                    for (var i=0;i<tbl.length;i++) {
                                        var v = Math.abs(parseFloat(tbl[i][1]) || 0.0)
                                        var angle = (total > 0) ? (v/total)*2*Math.PI : (2*Math.PI/tbl.length)
                                        var label = (tbl[i] && tbl[i].length>0) ? tbl[i][0] : ""
                                        var sliceSelected = true
                                        if (uiData && uiData._legendFilter && uiData._legendFilter.length > 0) sliceSelected = (uiData._legendFilter.indexOf(label) !== -1)
                                        ctx.beginPath(); ctx.moveTo(cx, cy); ctx.arc(cx, cy, radius, start, start+angle); ctx.closePath(); ctx.globalAlpha = sliceSelected ? 1.0 : 0.25; ctx.fillStyle = "hsl(" + ((i*360/tbl.length) % 360) + ",60%,60%)"; ctx.fill(); ctx.globalAlpha = 1.0; start += angle
                                    }
                                } else if (type === Analysis.chartTypes.histogram) {
                                    var months = []; var byContract = []; var byProperty = []; var totals = []
                                    for (var i=0;i<tbl.length;i++) { months.push(tbl[i][0]); try { var j = JSON.parse(tbl[i][1]); totals.push(parseFloat(j.total) || 0); byContract.push(j.byContract || {}); byProperty.push(j.byProperty || {}) } catch(e) { totals.push(0); byContract.push({}); byProperty.push({}) } }
                                    var maxv = 0; for (var i=0;i<totals.length;i++) maxv = Math.max(maxv, totals[i])
                                    var bw = width / Math.max(1, months.length)
                                    var categories = {}; for (var i=0;i<byContract.length;i++) for (var k in byContract[i]) categories[k]=true
                                    var catList = Object.keys(categories)
                                    for (var i=0;i<months.length;i++) {
                                        var x = i*bw + Analysis.render.compactBarTopOffset; var y0 = height - Analysis.render.histogramBottomPadding
                                        var stackedAlpha = 1.0 - plotCanvas.splitProgress
                                        if (stackedAlpha > 0) { ctx.globalAlpha = stackedAlpha; for (var ci=0; ci<catList.length; ++ci) { var cat = catList[ci]; var v = parseFloat(byContract[i][cat]) || 0; var h = (maxv>0) ? (v/maxv)*(height-Analysis.render.histogramTopPadding) : 0; var hStack = h * (1.0 - plotCanvas.splitProgress); ctx.fillStyle = page.colorForKey(cat); ctx.fillRect(x, y0 - hStack, bw - Analysis.render.histogramGroupPadding, hStack); y0 -= hStack } ctx.globalAlpha = 1.0 }
                                        var splitAlpha = plotCanvas.splitProgress
                                        if (splitAlpha > 0) {
                                            ctx.globalAlpha = splitAlpha
                                            var props = []
                                            for (var k in byProperty[i]) props.push({k:k,v:byProperty[i][k]})
                                            props.sort(function(a,b){ return b.v - a.v })
                                            var numProps = Math.max(1, props.length)
                                            var gw = Math.max(Analysis.render.compactBarMinWidth, Math.floor((bw-Analysis.render.histogramGroupPadding)/numProps))
                                            var totalPropsWidth = gw * props.length
                                            var startX = x + Math.floor(((bw - Analysis.render.histogramGroupPadding) - totalPropsWidth) / 2)

                                            var towerTops = []
                                            for (var pi=0; pi<props.length; ++pi) {
                                                var propName = props[pi].k
                                                var propVal = parseFloat(props[pi].v) || 0
                                                var xprop = startX + pi*gw
                                                var y0prop = height - Analysis.render.histogramBottomPadding

                                                var contractBreakdown = null
                                                if (byProperty[i] && byProperty[i][propName] && typeof byProperty[i][propName] === 'object') contractBreakdown = byProperty[i][propName]

                                                if (contractBreakdown) {
                                                    for (var ci=0; ci<catList.length; ++ci) {
                                                        var cat = catList[ci]
                                                        var v = parseFloat(contractBreakdown[cat]) || 0
                                                        var h = (maxv>0) ? (v/maxv)*(height-Analysis.render.histogramTopPadding) : 0
                                                        var hScaled = h * plotCanvas.splitProgress
                                                        ctx.fillStyle = page.colorForKey(cat)
                                                        ctx.fillRect(xprop, y0prop - hScaled, gw-Analysis.render.propertyBarInset, hScaled)
                                                        y0prop -= hScaled
                                                    }
                                                } else {
                                                    var totalContractsVal = 0
                                                    for (var ci=0; ci<catList.length; ++ci) totalContractsVal += parseFloat(byContract[i][catList[ci]]) || 0
                                                    for (var ci=0; ci<catList.length; ++ci) {
                                                        var cat = catList[ci]
                                                        var contractVal = parseFloat(byContract[i][cat]) || 0
                                                        var v = totalContractsVal > 0 ? (propVal * (contractVal / totalContractsVal)) : 0
                                                        var h = (maxv>0) ? (v/maxv)*(height-Analysis.render.histogramTopPadding) : 0
                                                        var hScaled = h * plotCanvas.splitProgress
                                                        ctx.fillStyle = page.colorForKey(cat)
                                                        ctx.fillRect(xprop, y0prop - hScaled, gw-Analysis.render.propertyBarInset, hScaled)
                                                        y0prop -= hScaled
                                                    }
                                                }

                                                towerTops.push(y0prop)
                                            }

                                            try {
                                                ctx.fillStyle = Theme.chartText
                                                ctx.font = Theme.fontSizeSmall + "px " + Analysis.render.chartFontFamily
                                                ctx.textBaseline = "top"
                                                    var maxLabelWidth = Math.max(Theme.spacing * 2, gw - 4)
                                                var hexRe = /^[0-9a-fA-F]{6,}$/
                                                for (var pi=0; pi<props.length; ++pi) {
                                                    var propId = props[pi].k || ""
                                                    var propName = propId
                                                    try { if (uiData && uiData.propertyName) propName = uiData.propertyName(propId) } catch(e) { }
                                                    if (hexRe.test(propName)) continue
                                                    var xprop = startX + pi*gw
                                                    var label = propName
                                                    var twp = ctx.measureText ? (ctx.measureText(label).width || 0) : 0
                                                    if (twp > maxLabelWidth) {
                                                        var ell = Analysis.text.ellipsis
                                                        var sub = label
                                                        while (ctx.measureText(sub + ell).width > maxLabelWidth && sub.length > 0) sub = sub.substring(0, sub.length-1)
                                                        label = sub + ell
                                                        twp = ctx.measureText(label).width
                                                    }
                                                    var centerX = xprop + ((gw - Analysis.render.propertyBarInset) / 2)
                                                    var lx = centerX - (twp / 2)
                                                    var towerTop = towerTops[pi] || (height - Analysis.render.histogramBottomPadding)
                                                    var ly = Math.min(height - (Theme.spacing * 3), Math.max(towerTop + Theme.margins, height - (Theme.spacing * 5)))
                                                    if (gw >= Analysis.render.propertyLabelMinWidth) ctx.fillText(label, Math.max(xprop, lx), ly)
                                                }
                                            } catch(e) {}

                                            ctx.globalAlpha = 1.0
                                        }
                                        try { var totalVal = totals[i] || 0; ctx.fillStyle = Theme.shadow; ctx.font = Theme.fontSize + "px " + Analysis.render.chartFontFamily; var txt = totalVal.toFixed(2); var tw = ctx.measureText ? (ctx.measureText(txt).width || 0) : 0; var txv = x + ((bw - Analysis.render.histogramGroupPadding) / 2) - (tw/2); var hTotal = (maxv>0) ? (totals[i]/maxv)*(height-Analysis.render.histogramTopPadding) : 0; var labelYv = Math.max(Theme.chartPanelSpacing, height - Theme.spacingLarge - hTotal); ctx.textBaseline = "bottom"; ctx.fillText(txt, Math.max(x, txv), labelYv) } catch(e) {}
                                        ctx.fillStyle = Theme.placeholderText; try { var label = Analysis.formatMonthLabel(months[i]); var fm = ctx.measureText ? ctx.measureText(label) : null; var tx = x + ((bw - Analysis.render.histogramGroupPadding) / 2) - (fm ? (fm.width/2) : 0); try { ctx.font = Theme.fontSizeSmall + "px " + Analysis.render.chartFontFamily } catch(e) {} ctx.textBaseline = "top"; ctx.fillText(label, Math.max(x, tx), height - Theme.spacingMedium - Theme.margins) } catch(e) { ctx.fillText(months[i], x, height + Theme.spacingMedium) }
                                    }
                                }
                            }
                            Component.onCompleted: { if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start(); }
                            onWidthChanged: { if (width >= Analysis.layout.minDebugRepaintWidth && height >= Analysis.layout.minDebugRepaintWidth) plotCanvas.requestPaint(); else if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() }
                            onHeightChanged: { if (width >= Analysis.layout.minDebugRepaintWidth && height >= Analysis.layout.minDebugRepaintWidth) plotCanvas.requestPaint(); else if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() }
                            Connections { target: uiData; function onLastAnalysisResultChanged() { try { rebuildHistLegend(); if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() } catch(e) {} } }
                        }
                    }

                    RowLayout { Layout.fillWidth: true; spacing: Theme.spacingMedium
                        Item { Layout.fillWidth: true }
                        ColumnLayout { Layout.preferredWidth: Analysis.layout.splitControlsWidth; spacing: Theme.chartPanelSpacing; Layout.alignment: Qt.AlignRight
                            RowLayout { spacing: Theme.chartPanelSpacing; Layout.fillWidth: true; visible: Analysis.shouldShowHistogramControls(uiData ? uiData.lastAnalysisResult : null)
                                Label { text: qsTr("") }
                            }
                            RowLayout { spacing: Theme.chartPanelSpacing; Layout.fillWidth: true; visible: Analysis.shouldShowHistogramControls(uiData ? uiData.lastAnalysisResult : null)
                                Label { text: qsTr("Split by property") }
                                Switch { id: splitByPropertySwitch; onCheckedChanged: { plotCanvas.splitProgress = checked ? 1.0 : 0.0; rebuildHistLegend() } }
                            }
                        }
                    }

                    ColumnLayout { id: legendCol; Layout.fillWidth: true; spacing: Theme.chartPanelSpacing; Layout.topMargin: Analysis.layout.legendTopMargin
                        Label { text: qsTr("Legend") }
                        Flickable { id: histLegendFlick; Layout.fillWidth: true; Layout.preferredHeight: Theme.chartLegendHeight; contentHeight: histLegendRepeater.implicitHeight; clip: true; visible: Analysis.shouldShowHistogramLegend(uiData ? uiData.lastAnalysisResult : null, histLegendModel)
                            Column { id: histLegendList; width: parent.width
                                Repeater { id: histLegendRepeater; model: histLegendModel
                                    delegate: RowLayout { spacing: Theme.spacingMedium; width: parent.width
                                        Rectangle { width: Theme.chartLegendMarkerSize; height: Theme.chartLegendMarkerSize; color: page.colorForKey(model.name) }
                                        Label { text: (model && model.name) ? model.name : ""; Layout.fillWidth: true }
                                        Label { text: (model && model.value) ? model.value.toFixed(2) : Analysis.text.defaultLegendValue; Layout.preferredWidth: Theme.chartValueLabelWidth }
                                        Label { text: (model && model.value && histLegendTotal>0) ? ((model.value / histLegendTotal * 100).toFixed(1) + Analysis.text.percentSuffix) : ""; Layout.preferredWidth: Theme.chartPercentLabelWidth }
                                    }
                                }
                            }
                        }
                        RowLayout { Layout.fillWidth: true; spacing: Theme.chartPanelSpacing; visible: (function(){ if (!propListModel || propListModel.length === 0) return false; return true })()
                            Repeater { model: propListModel
                                delegate: Rectangle { radius: Theme.radius + Theme.margins; color: "transparent"; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; height: Theme.chartPropertyChipHeight; width: Math.max(Theme.chartPropertyChipMinWidth, childrenRect.width + (Theme.spacing + Theme.margins * 2))
                                    RowLayout { anchors.fill: parent; anchors.margins: Theme.margins * 2; spacing: Theme.spacingSmall
                                        Label { text: (model && model.name) ? model.name : "" }
                                        Label { text: (model && model.value) ? (model.value.toFixed(2)) : ""; font.pixelSize: Theme.fontSizeSmall; color: Theme.placeholderText }
                                    }
                                }
                            }
                        }
                        Flickable { Layout.fillWidth: true; Layout.preferredHeight: Theme.chartLegendHeight; contentHeight: legendList.implicitHeight; clip: true; visible: Analysis.shouldShowNonHistogramLegend(uiData ? uiData.lastAnalysisResult : null)
                            Column { id: legendList; width: parent.width
                                Repeater { model: (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []
                                    delegate: RowLayout { spacing: Theme.spacingMedium; width: parent.width
                                        Rectangle { width: Theme.chartLegendMarkerSize; height: Theme.chartLegendMarkerSize; color: Qt.hsla(((index) / Math.max(1, ((uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table.length : 1))), 0.6, 0.6, 1.0) }
                                        Button {
                                            id: legendBtn
                                            text: (modelData && modelData.length>0) ? modelData[0] : ""
                                            Layout.fillWidth: true
                                            opacity: (uiData && uiData._legendFilter && uiData._legendFilter.indexOf(text) !== -1) ? 1.0 : 0.7
                                            onClicked: {
                                                var name = text
                                                if (!uiData) return
                                                if (!uiData._legendFilter) uiData._legendFilter = []
                                                var idx = uiData._legendFilter.indexOf(name)
                                                if (idx === -1) uiData._legendFilter.push(name); else uiData._legendFilter.splice(idx,1)
                                                plotCanvas.requestPaint()
                                            }
                                        }
                                        Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return v.toFixed(2); })(); Layout.preferredWidth: Theme.chartValueLabelWidth }
                                        Label { text: (function(){ var total = 0; var tbl = (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []; for (var i=0;i<tbl.length;i++) total += Math.abs(parseFloat(tbl[i][1])||0); var v = (modelData && modelData.length>1) ? Math.abs(parseFloat(modelData[1])||0) : 0; var pct = total>0 ? (v/total*100) : 0; return pct.toFixed(1) + Analysis.text.percentSuffix; })(); Layout.preferredWidth: Theme.chartPercentLabelWidth }
                                    }
                                }
                            }
                        }
                    }
                }
                    }

            }
        }
    }


