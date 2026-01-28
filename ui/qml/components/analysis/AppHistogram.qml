import QtQuick 2.15
import QtQuick.Controls 2.15
pragma NativeMethodBehavior: AcceptThisObject

Item {
    id: root
    property var table: []
    property var uiData
    property var legendFilter: []
    property var propertyNameForId
    property real splitProgress: 0.0
    Behavior on splitProgress { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
    // sizing is controlled by the parent layout; do not anchor to parent here

    function colorForKey(k) {
        try {
            var palette = ["#8dd3c7","#ffffb3","#bebada","#fb8072","#80b1d3","#fdb462","#b3de69","#fccde5","#d9d9d9","#bc80bd","#ccebc5","#ffed6f"]
            var h = 0
            if (!k) return palette[0]
            for (var i=0;i<k.length;i++) { h = ((h<<5)-h) + k.charCodeAt(i); h |= 0 }
            return palette[Math.abs(h) % palette.length]
        } catch(e) { return "#888" }
    }

    Canvas {
        id: histCanvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d"); ctx.reset(); ctx.clearRect(0,0,width,height);
            if (!root.visible || width < 50 || !table || table.length === 0) return;
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
                var x = i*bw + 4; var y0 = height - 18
                // stacked columns for contracts - base bars collapse as splitProgress increases
                var baseline = height - 18
                for (var ci=0; ci<catList.length; ++ci) {
                    var cat = catList[ci]
                    var v = parseFloat(byContract[i][cat]) || 0
                    var h = (maxv>0) ? (v/maxv)*(height-30) : 0
                    var hBase = h * (1 - root.splitProgress)
                    ctx.fillStyle = colorForKey(cat)
                    ctx.fillRect(x, baseline - hBase, bw - 8, hBase)
                    baseline -= hBase
                }

                // if splitProgress > 0, overlay property towers
                if (root.splitProgress > 0) {
                    var props = []
                    for (var k in byProperty[i]) props.push({k:k,v:byProperty[i][k]})
                    props.sort(function(a,b){ return b.v - a.v })
                    var numProps = Math.max(1, props.length)
                    var gw = Math.max(2, Math.floor((bw-8)/numProps))
                    var totalPropsWidth = gw * props.length
                    var startX = x + Math.floor(((bw - 8) - totalPropsWidth) / 2)
                    for (var pi=0; pi<props.length; ++pi) {
                        var propName = props[pi].k
                        var propVal = parseFloat(props[pi].v) || 0
                        var xprop = startX + pi*gw
                        var y0prop = height - 18
                        var totalContractsVal = 0
                        for (var ci=0; ci<catList.length; ++ci) totalContractsVal += parseFloat(byContract[i][catList[ci]]) || 0
                        for (var ci=0; ci<catList.length; ++ci) {
                            var cat = catList[ci]
                            var contractVal = parseFloat(byContract[i][cat]) || 0
                            var v = totalContractsVal > 0 ? (propVal * (contractVal / totalContractsVal)) : 0
                            var h = (maxv>0) ? (v/maxv)*(height-30) : 0
                            var hScaled = h * root.splitProgress
                            ctx.fillStyle = colorForKey(cat)
                            ctx.fillRect(xprop, y0prop - hScaled, gw-2, hScaled)
                            y0prop -= hScaled
                        }
                        // draw property label under tower
                        try {
                            ctx.fillStyle = "#333"
                            ctx.font = "11px sans-serif"
                            ctx.textBaseline = "top"
                            var label = props[pi].k || ""
                            var propName = label
                            try { if (propertyNameForId) propName = propertyNameForId(label) } catch(e) {}
                            var tw = ctx.measureText ? (ctx.measureText(propName).width || 0) : 0
                            var lx = xprop + ((gw - 2) / 2) - (tw / 2)
                            if (gw >= 36) ctx.fillText(propName, Math.max(xprop, lx), height - 36)
                        } catch(e) {}
                    // reset globalAlpha after overlay
                    ctx.globalAlpha = 1.0
                    }
                    // ensure repaint on split progress change
                    try { if (root.splitProgress !== undefined) {} } catch(e) {}
                }
                // draw month label centered under the column
                try {
                    ctx.fillStyle = "#666"
                    ctx.font = "11px sans-serif"
                    ctx.textBaseline = "top"
                    var label = months[i]
                    var tw = ctx.measureText ? (ctx.measureText(label).width || 0) : 0
                    var lx = x + ((bw - 8) / 2) - (tw / 2)
                    ctx.fillText(label, Math.max(x, lx), height - 14)
                } catch(e) {}
            }
        }

    }

    // single requestPaint API
    function requestPaint() { try { histCanvas.requestPaint(); } catch(e) {} }
    onTableChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }
    Component.onCompleted: { try { if (table && table.length>0) requestPaint(); } catch(e) {} }
    onSplitProgressChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }

}
