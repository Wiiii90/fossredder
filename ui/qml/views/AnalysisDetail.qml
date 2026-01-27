import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    id: page
    property var stackView
    property var histLegendModel: []
    property real histLegendTotal: 0
    property var propListModel: []
    // deterministic color mapping for keys (contract types)
    function hashString(s) {
        var h = 0;
        if (!s) return 0;
        for (var i=0;i<s.length;i++) {
            h = ((h<<5) - h) + s.charCodeAt(i);
            h |= 0;
        }
        return Math.abs(h);
    }
    function colorForKey(k) {
        try {
            // softer qualitative palette (ColorBrewer / pastel-like)
            var palette = ["#8dd3c7","#ffffb3","#bebada","#fb8072","#80b1d3","#fdb462","#b3de69","#fccde5","#d9d9d9","#bc80bd","#ccebc5","#ffed6f"]
            var idx = hashString(k) % palette.length
            return palette[idx]
        } catch(e) { return "#888" }
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
        } catch(e) { console.log('rebuildHistLegend error', e) }
    }
    // hist legend state and helper defined on page root
    
    width: stackView ? stackView.width : (parent ? parent.width : 800)
    height: stackView ? stackView.height : (parent ? parent.height : 600)
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        
        Component.onCompleted: { console.log("AnalysisDetail: page width=", page.width, "height=", page.height) }
        onWidthChanged: { if (width < 80) console.log("AnalysisDetail: small page width=", width) }
        Connections { target: uiData; function onLastAnalysisResultChanged() { try { rebuildHistLegend(); } catch(e) {} } }

            RowLayout { Layout.fillWidth: true
            AppButton { text: qsTr("Back"); onClicked: { if (stackView) stackView.pop() } }
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Recompute"); onClicked: {
                if (!uiDomain || !uiData || !uiData.selectedAnalysis) return
                var aid = uiData.selectedAnalysis.id
                var res = uiDomain.computeAnalysis(aid, uiData.selectedAnalysis.filterSpec ? uiData.selectedAnalysis.filterSpec : "")
                try { uiData.lastAnalysisResult = res; rebuildHistLegend(); } catch(e) {}
            } }
        }

        GroupBox { title: qsTr("Preview"); Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout { anchors.fill: parent; anchors.margins: 6; spacing: 6
                // timer to defer initial canvas painting until layout stabilizes
                Timer { id: initialCanvasPaintTimer; interval: 300; repeat: true; running: false; property int attempts: 0
                    onTriggered: {
                        try {
                            attempts++;
                            // if canvas has usable width, paint and stop
                            if (plotCanvas.width >= 120 && page.visible) {
                                plotCanvas.requestPaint(); rebuildHistLegend(); attempts = 0; stop();
                                return
                            }
                            // give up after 6 attempts and paint anyway to show something
                            if (attempts >= 6) { plotCanvas.requestPaint(); attempts = 0; stop(); }
                        } catch(e) { attempts = 0; stop(); }
                    }
                }
                // Metrics and debug row removed (was only used for debugging)

                // Simple plot preview (pie or histogram) with legend below
                ColumnLayout { Layout.fillWidth: true; spacing: 6
                    // Canvas area: fills the available width and has fixed preferred height
                    Item { Layout.fillWidth: true; Layout.preferredHeight: 320; Layout.minimumHeight: 200
                        Canvas {
                            id: plotCanvas
                            property real splitProgress: 0.0
                            property int repaintAttempts: 0
                            Behavior on splitProgress { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
                            anchors.fill: parent
                            focus: true
                            Timer { id: repaintTimer; interval: 100; repeat: false; onTriggered: plotCanvas.requestPaint() }
                            onSplitProgressChanged: { plotCanvas.requestPaint(); }
                            onPaint: {
                                // drawing logic unchanged
                                if (width >= 50) console.log("PlotCanvas:onPaint", "w=", width, "h=", height, "rows=", (uiData && uiData.lastAnalysisResult && uiData.lastAnalysisResult.table)? uiData.lastAnalysisResult.table.length : 0)
                                var ctx = getContext("2d")
                                ctx.reset()
                                ctx.clearRect(0,0,width,height)
                                if (!page.visible || width < 100) { if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start(); return }
                                if (!uiData || !uiData.lastAnalysisResult) return
                                plotCanvas.repaintAttempts = 0
                                var tbl = uiData.lastAnalysisResult.table
                                if (!tbl || tbl.length === 0) return
                                var type = ""
                                if (uiData.lastAnalysisResult && uiData.lastAnalysisResult.type) type = uiData.lastAnalysisResult.type
                                else if (uiData.lastAnalysisResult && uiData.lastAnalysisResult.config) { try { var cfg = JSON.parse(uiData.lastAnalysisResult.config); if (cfg && cfg.plotType) type = cfg.plotType } catch(e) {} }
                                if (!type && tbl && tbl.length > 0) type = "pie"
                                if (width <= 0 || height <= 0) return
                                var cx = width/2; var cy = height/2
                                if (type === "pie") {
                                    var total = 0.0
                                    for (var i=0;i<tbl.length;i++) total += Math.abs(parseFloat(tbl[i][1]) || 0.0)
                                    var start = -Math.PI/2
                                    var radius = Math.min(width, height)/2 - 10
                                    if (radius <= 0) {
                                        var maxv = 0
                                        for (var i=0;i<tbl.length;i++) maxv = Math.max(maxv, Math.abs(parseFloat(tbl[i][1])||0))
                                        for (var i=0;i<tbl.length;i++) { var v = Math.abs(parseFloat(tbl[i][1])||0); var bw = Math.max(2, Math.floor((width-40) * (v / Math.max(1, maxv)))); ctx.fillStyle = "hsl(" + ((i*360/tbl.length) % 360) + ",60%,60%)"; ctx.fillRect(20, i*18 + 4, bw, 12) }
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
                                } else if (type === "histogram") {
                                    // histogram drawing (kept as before)
                                    var months = []; var byContract = []; var byProperty = []; var totals = []
                                    for (var i=0;i<tbl.length;i++) { months.push(tbl[i][0]); try { var j = JSON.parse(tbl[i][1]); totals.push(parseFloat(j.total) || 0); byContract.push(j.byContract || {}); byProperty.push(j.byProperty || {}) } catch(e) { totals.push(0); byContract.push({}); byProperty.push({}) } }
                                    var maxv = 0; for (var i=0;i<totals.length;i++) maxv = Math.max(maxv, totals[i])
                                    var bw = width / Math.max(1, months.length)
                                    var categories = {}; for (var i=0;i<byContract.length;i++) for (var k in byContract[i]) categories[k]=true
                                    var catList = Object.keys(categories)
                                    for (var i=0;i<months.length;i++) {
                                        var x = i*bw + 4; var y0 = height - 18
                                        var stackedAlpha = 1.0 - plotCanvas.splitProgress
                                        if (stackedAlpha > 0) { ctx.globalAlpha = stackedAlpha; for (var ci=0; ci<catList.length; ++ci) { var cat = catList[ci]; var v = parseFloat(byContract[i][cat]) || 0; var h = (maxv>0) ? (v/maxv)*(height-30) : 0; var hStack = h * (1.0 - plotCanvas.splitProgress); ctx.fillStyle = page.colorForKey(cat); ctx.fillRect(x, y0 - hStack, bw - 8, hStack); y0 -= hStack } ctx.globalAlpha = 1.0 }
                                        var splitAlpha = plotCanvas.splitProgress
                                        if (splitAlpha > 0) {
                                            ctx.globalAlpha = splitAlpha
                                            // build properties list for this month and center towers inside month column
                                            var props = []
                                            for (var k in byProperty[i]) props.push({k:k,v:byProperty[i][k]})
                                            props.sort(function(a,b){ return b.v - a.v })
                                            var numProps = Math.max(1, props.length)
                                            var gw = Math.max(2, Math.floor((bw-8)/numProps))
                                            var totalPropsWidth = gw * props.length
                                            var startX = x + Math.floor(((bw - 8) - totalPropsWidth) / 2)

                                            // for each property draw a tower subdivided by contract types (centered)
                                            var towerTops = []
                                            for (var pi=0; pi<props.length; ++pi) {
                                                var propName = props[pi].k
                                                var propVal = parseFloat(props[pi].v) || 0
                                                var xprop = startX + pi*gw
                                                var y0prop = height - 18

                                                // check if byProperty contains a per-contract breakdown for this property
                                                var contractBreakdown = null
                                                if (byProperty[i] && byProperty[i][propName] && typeof byProperty[i][propName] === 'object') contractBreakdown = byProperty[i][propName]

                                                if (contractBreakdown) {
                                                    // use explicit contract breakdown values
                                                    for (var ci=0; ci<catList.length; ++ci) {
                                                        var cat = catList[ci]
                                                        var v = parseFloat(contractBreakdown[cat]) || 0
                                                        var h = (maxv>0) ? (v/maxv)*(height-30) : 0
                                                        var hScaled = h * plotCanvas.splitProgress
                                                        ctx.fillStyle = page.colorForKey(cat)
                                                        ctx.fillRect(xprop, y0prop - hScaled, gw-2, hScaled)
                                                        y0prop -= hScaled
                                                    }
                                                } else {
                                                    // distribute property value proportionally to month contract distribution
                                                    var totalContractsVal = 0
                                                    for (var ci=0; ci<catList.length; ++ci) totalContractsVal += parseFloat(byContract[i][catList[ci]]) || 0
                                                    for (var ci=0; ci<catList.length; ++ci) {
                                                        var cat = catList[ci]
                                                        var contractVal = parseFloat(byContract[i][cat]) || 0
                                                        var v = totalContractsVal > 0 ? (propVal * (contractVal / totalContractsVal)) : 0
                                                        var h = (maxv>0) ? (v/maxv)*(height-30) : 0
                                                        var hScaled = h * plotCanvas.splitProgress
                                                        ctx.fillStyle = page.colorForKey(cat)
                                                        ctx.fillRect(xprop, y0prop - hScaled, gw-2, hScaled)
                                                        y0prop -= hScaled
                                                    }
                                                }

                                                // record top of this tower for label placement
                                                towerTops.push(y0prop)
                                            }

                                            // draw property labels centered under each property tower (wrap if too long)
                                            try {
                                                ctx.fillStyle = "#333"
                                                ctx.font = "11px sans-serif"
                                                ctx.textBaseline = "top"
                                                var maxLabelWidth = Math.max(24, gw - 4)
                                                var hexRe = /^[0-9a-fA-F]{6,}$/
                                                for (var pi=0; pi<props.length; ++pi) {
                                                    var propId = props[pi].k || ""
                                                    var propName = propId
                                                    try { if (uiData && uiData.propertyNameForId) propName = uiData.propertyNameForId(propId) } catch(e) { }
                                                    // if name looks like a hex id, skip canvas label (use pills below)
                                                    if (hexRe.test(propName)) continue
                                                    var xprop = startX + pi*gw
                                                    var label = propName
                                                    var twp = ctx.measureText ? (ctx.measureText(label).width || 0) : 0
                                                    if (twp > maxLabelWidth) {
                                                        // truncate with ellipsis
                                                        var ell = "..."
                                                        var sub = label
                                                        while (ctx.measureText(sub + ell).width > maxLabelWidth && sub.length > 0) sub = sub.substring(0, sub.length-1)
                                                        label = sub + ell
                                                        twp = ctx.measureText(label).width
                                                    }
                                                    var centerX = xprop + ((gw - 2) / 2)
                                                    var lx = centerX - (twp / 2)
                                                    // place property label just below the tower top or fallback below totals
                                                    var towerTop = towerTops[pi] || (height - 18)
                                                    var ly = Math.min(height - 36, Math.max(towerTop + 2, height - 60))
                                                    if (gw >= 36) ctx.fillText(label, Math.max(xprop, lx), ly)
                                                }
                                            } catch(e) {}

                                            ctx.globalAlpha = 1.0
                                        }
                                        try { var totalVal = totals[i] || 0; ctx.fillStyle = "#000"; ctx.font = "12px sans-serif"; var txt = totalVal.toFixed(2); var tw = ctx.measureText ? (ctx.measureText(txt).width || 0) : 0; var txv = x + ((bw - 8) / 2) - (tw/2); var hTotal = (maxv>0) ? (totals[i]/maxv)*(height-30) : 0; var labelYv = Math.max(6, height - 20 - hTotal); ctx.textBaseline = "bottom"; ctx.fillText(txt, Math.max(x, txv), labelYv) } catch(e) {}
                                        ctx.fillStyle = "#666"; try { function formatMonthLabel(m) { if (!m) return m; var parts = m.split('-'); if (parts.length >= 2 && parts[0].length==4) { var yy = parts[0]; var mm = parts[1]; var names = ["January","February","March","April","May","June","July","August","September","October","November","December"]; var mi = parseInt(mm,10)-1; if (!isNaN(mi) && mi>=0 && mi<12) return names[mi] + " " + yy } return m } var label = formatMonthLabel(months[i]); var fm = ctx.measureText ? ctx.measureText(label) : null; var tx = x + ((bw - 8) / 2) - (fm ? (fm.width/2) : 0); try { ctx.font = "11px sans-serif" } catch(e) {} ctx.textBaseline = "top"; ctx.fillText(label, Math.max(x, tx), height - 14) } catch(e) { ctx.fillText(months[i], x, height + 8) }
                                    }
                                }
                            }
                            Component.onCompleted: { if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start(); }
                            onWidthChanged: { if (width >= 50 && height >= 50) plotCanvas.requestPaint(); else if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() }
                            onHeightChanged: { if (width >= 50 && height >= 50) plotCanvas.requestPaint(); else if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() }
                            Connections { target: uiData; function onLastAnalysisResultChanged() { try { rebuildHistLegend(); if (!initialCanvasPaintTimer.running) initialCanvasPaintTimer.start() } catch(e) {} } }
                        }
                    }

                    // Options row: split toggle (right-aligned)
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Item { Layout.fillWidth: true }
                        ColumnLayout { Layout.preferredWidth: 220; spacing: 6; Layout.alignment: Qt.AlignRight
                            RowLayout { spacing: 6; Layout.fillWidth: true; visible: (function(){ var t=""; if (!uiData || !uiData.lastAnalysisResult) return false; if (uiData.lastAnalysisResult.type) t=uiData.lastAnalysisResult.type; else if (uiData.lastAnalysisResult.config) { try { var c = JSON.parse(uiData.lastAnalysisResult.config); if (c && c.plotType) t = c.plotType } catch(e) {} } return t === "histogram"; })()
                                Label { text: qsTr("") }
                            }
                            RowLayout { spacing: 6; Layout.fillWidth: true; visible: (function(){ var t=""; if (!uiData || !uiData.lastAnalysisResult) return false; if (uiData.lastAnalysisResult.type) t=uiData.lastAnalysisResult.type; else if (uiData.lastAnalysisResult.config) { try { var c = JSON.parse(uiData.lastAnalysisResult.config); if (c && c.plotType) t = c.plotType } catch(e) {} } return t === "histogram"; })()
                                Label { text: qsTr("Split by property") }
                                Switch { id: splitByPropertySwitch; onCheckedChanged: { plotCanvas.splitProgress = checked ? 1.0 : 0.0; rebuildHistLegend() } }
                            }
                        }
                    }

                    // Legend area below the plot
                    ColumnLayout { id: legendCol; Layout.fillWidth: true; spacing: 6; Layout.topMargin: 8
                        Label { text: qsTr("Legend") }
                        Flickable { id: histLegendFlick; Layout.fillWidth: true; Layout.preferredHeight: 120; contentHeight: histLegendRepeater.implicitHeight; clip: true; visible: (function(){ if (!uiData || !uiData.lastAnalysisResult) return false; var t = uiData.lastAnalysisResult.type ? uiData.lastAnalysisResult.type : (uiData.lastAnalysisResult.config ? (function(){ try { var c = JSON.parse(uiData.lastAnalysisResult.config); return c.plotType } catch(e){ return "" } })() : ""); return t === "histogram" || (histLegendModel && histLegendModel.length>0); })()
                            Column { id: histLegendList; width: parent.width
                                Repeater { id: histLegendRepeater; model: histLegendModel
                                    delegate: RowLayout { spacing: 8; width: parent.width
                                        Rectangle { width: 12; height: 12; color: page.colorForKey(model.name) }
                                        Label { text: (model && model.name) ? model.name : ""; Layout.fillWidth: true }
                                        Label { text: (model && model.value) ? model.value.toFixed(2) : "0.00"; Layout.preferredWidth: 80 }
                                        Label { text: (model && model.value && histLegendTotal>0) ? ((model.value / histLegendTotal * 100).toFixed(1) + "%") : ""; Layout.preferredWidth: 60 }
                                    }
                                }
                            }
                        }
                        // properties row: show property names as pills (no color) when histogram or when properties exist
                        RowLayout { Layout.fillWidth: true; spacing: 6; visible: (function(){ if (!propListModel || propListModel.length === 0) return false; return true })()
                            Repeater { model: propListModel
                                delegate: Rectangle { radius: 6; color: "transparent"; border.color: "#ccc"; border.width: 1; height: 22; width: Math.max(80, childrenRect.width + 16)
                                    RowLayout { anchors.fill: parent; anchors.margins: 4; spacing: 6
                                        Label { text: (model && model.name) ? model.name : "" }
                                        Label { text: (model && model.value) ? (model.value.toFixed(2)) : ""; font.pixelSize: 11; color: "#666" }
                                    }
                                }
                            }
                        }
                        // pie/table legend fallback (unchanged colors based on index)
                        Flickable { Layout.fillWidth: true; Layout.preferredHeight: 120; contentHeight: legendList.implicitHeight; clip: true; visible: (function(){ if (!uiData || !uiData.lastAnalysisResult) return false; var t = uiData.lastAnalysisResult.type ? uiData.lastAnalysisResult.type : (uiData.lastAnalysisResult.config ? (function(){ try { var c = JSON.parse(uiData.lastAnalysisResult.config); return c.plotType } catch(e){ return "" } })() : ""); return t !== "histogram"; })()
                            Column { id: legendList; width: parent.width
                                Repeater { model: (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []
                                    delegate: RowLayout { spacing: 8; width: parent.width
                                        Rectangle { width: 12; height: 12; color: Qt.hsla(((index) / Math.max(1, ((uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table.length : 1))), 0.6, 0.6, 1.0) }
                                        Button {
                                            id: legendBtn
                                            text: (modelData && modelData.length>0) ? modelData[0] : ""
                                            Layout.fillWidth: true
                                            // show selected state by opacity
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
                                        Label { text: (function(){ var v = (modelData && modelData.length>1) ? parseFloat(modelData[1])||0 : 0; return v.toFixed(2); })(); Layout.preferredWidth: 80 }
                                        Label { text: (function(){ var total = 0; var tbl = (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []; for (var i=0;i<tbl.length;i++) total += Math.abs(parseFloat(tbl[i][1])||0); var v = (modelData && modelData.length>1) ? Math.abs(parseFloat(modelData[1])||0) : 0; var pct = total>0 ? (v/total*100) : 0; return pct.toFixed(1) + "%"; })(); Layout.preferredWidth: 60 }
                                    }
                                }
                            }
                        }
                    }
                }
                    }

                // Matched transactions removed - list view intentionally omitted
            }
        }
    }
