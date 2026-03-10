import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0
import "../../Constants/Analysis.js" as Analysis
pragma NativeMethodBehavior: AcceptThisObject

Item {
    id: root
    property var table: []
    property var uiData
    property var legendFilter: []
    property var propertyNameForId
    property real splitProgress: 0.0
    Behavior on splitProgress { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }

    function colorForKey(k) {
        return Analysis.colorForKey(k, Theme.analysis.palette, Theme.chartFallback)
    }

    Canvas {
        id: histCanvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d"); ctx.reset(); ctx.clearRect(0,0,width,height);
            if (!root.visible || width < Theme.analysis.layout.minDebugRepaintWidth || !table || table.length === 0) return;
            var months = [];
            var totals = [];
            var byContract = [];
            var byProperty = [];
            for (var i=0;i<table.length;i++) {
                months.push(table[i][0]);
                try { var j = JSON.parse(table[i][1]); totals.push(parseFloat(j.total)||0); byContract.push(j.byContract||{}); byProperty.push(j.byProperty||{}) } catch(e) { totals.push(0); byContract.push({}); byProperty.push({}) }
            }
            var maxv = 0; for (var i=0;i<totals.length;i++) maxv = Math.max(maxv, totals[i]);
            var bw = width / Math.max(1, months.length);
            var categories = {};
            for (var i=0;i<byContract.length;i++) for (var k in byContract[i]) categories[k]=true
            var catList = Object.keys(categories)

            for (var i=0;i<months.length;i++) {
                var x = i*bw + Theme.analysis.render.compactBarTopOffset; var y0 = height - Theme.analysis.render.histogramBottomPadding
                var baseline = height - Theme.analysis.render.histogramBottomPadding
                for (var ci=0; ci<catList.length; ++ci) {
                    var cat = catList[ci]
                    var v = parseFloat(byContract[i][cat]) || 0
                    var h = (maxv>0) ? (v/maxv)*(height-Theme.analysis.render.histogramTopPadding) : 0
                    var hBase = h * (1 - root.splitProgress)
                    ctx.fillStyle = colorForKey(cat)
                    ctx.fillRect(x, baseline - hBase, bw - Theme.analysis.render.histogramGroupPadding, hBase)
                    baseline -= hBase
                }

                if (root.splitProgress > 0) {
                    var props = []
                    for (var k in byProperty[i]) props.push({k:k,v:byProperty[i][k]})
                    props.sort(function(a,b){ return b.v - a.v })
                    var numProps = Math.max(1, props.length)
                    var gw = Math.max(Theme.analysis.render.compactBarMinWidth, Math.floor((bw-Theme.analysis.render.histogramGroupPadding)/numProps))
                    var totalPropsWidth = gw * props.length
                    var startX = x + Math.floor(((bw - Theme.analysis.render.histogramGroupPadding) - totalPropsWidth) / 2)
                    for (var pi=0; pi<props.length; ++pi) {
                        var propName = props[pi].k
                        var propVal = parseFloat(props[pi].v) || 0
                        var xprop = startX + pi*gw
                        var y0prop = height - Theme.analysis.render.histogramBottomPadding
                        var totalContractsVal = 0
                        for (var ci=0; ci<catList.length; ++ci) totalContractsVal += parseFloat(byContract[i][catList[ci]]) || 0
                        for (var ci=0; ci<catList.length; ++ci) {
                            var cat = catList[ci]
                            var contractVal = parseFloat(byContract[i][cat]) || 0
                            var v = totalContractsVal > 0 ? (propVal * (contractVal / totalContractsVal)) : 0
                            var h = (maxv>0) ? (v/maxv)*(height-Theme.analysis.render.histogramTopPadding) : 0
                            var hScaled = h * root.splitProgress
                            ctx.fillStyle = colorForKey(cat)
                            ctx.fillRect(xprop, y0prop - hScaled, gw-Theme.analysis.render.propertyBarInset, hScaled)
                            y0prop -= hScaled
                        }
                        try {
                            ctx.fillStyle = Theme.chartText
                            ctx.font = Theme.fontSizeSmall + "px " + Theme.fontFamily
                            ctx.textBaseline = "top"
                            var label = props[pi].k || ""
                            var propName = label
                            try { if (propertyNameForId) propName = propertyNameForId(label) } catch(e) {}
                            var tw = ctx.measureText ? (ctx.measureText(propName).width || 0) : 0
                            var lx = xprop + ((gw - Theme.analysis.render.propertyBarInset) / 2) - (tw / 2)
                            if (gw >= Theme.analysis.render.propertyLabelMinWidth) ctx.fillText(propName, Math.max(xprop, lx), height - (Theme.spacing * 3))
                        } catch(e) {}
                    ctx.globalAlpha = 1.0
                    }
                    try { if (root.splitProgress !== undefined) {} } catch(e) {}
                }
                try {
                    ctx.fillStyle = Theme.placeholderText
                    ctx.font = Theme.fontSizeSmall + "px " + Theme.fontFamily
                    ctx.textBaseline = "top"
                    var label = months[i]
                    var tw = ctx.measureText ? (ctx.measureText(label).width || 0) : 0
                    var lx = x + ((bw - Theme.analysis.render.histogramGroupPadding) / 2) - (tw / 2)
                    ctx.fillText(label, Math.max(x, lx), height - (Theme.spacingMedium + Theme.margins))
                } catch(e) {}
            }
        }

    }

    function requestPaint() { try { histCanvas.requestPaint(); } catch(e) {} }
    onTableChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }
    Component.onCompleted: { try { if (table && table.length>0) requestPaint(); } catch(e) {} }
    onSplitProgressChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }

}
