import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0
import "../../Constants/Analysis.js" as Analysis

Item {
    id: root
    property var table: []
    property var legendFilter: []
    property var propertyNameForId
    implicitWidth: 320
    implicitHeight: 320
    onTableChanged: { try { if (pieCanvas) pieCanvas.requestPaint(); } catch(e) {} }

    function colorForKey(k, i) {
        try {
            if (k) return Analysis.colorForKey(k, Theme.analysis.palette, Theme.chartFallback)
            return Theme.analysis.palette[i % Theme.analysis.palette.length]
        } catch(e) { return Theme.chartFallback }
    }

    Canvas {
        id: pieCanvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d"); ctx.reset(); ctx.clearRect(0,0,width,height)
            if (!root.visible || width < Theme.analysis.layout.minDebugRepaintWidth || !table || table.length === 0) return

            var total = 0
            for (var i=0;i<table.length;i++) total += Math.abs(parseFloat(table[i][1])||0)
            var start = Theme.analysis.render.pieStartAngle
            var radius = Math.min(width, height)/2 - Theme.analysis.render.pieRadiusPadding
            var cx = width/2; var cy = height/2
            if (radius <= 0) {
                var maxv = 0
                for (var i=0;i<table.length;i++) maxv = Math.max(maxv, Math.abs(parseFloat(table[i][1])||0))
                for (var i=0;i<table.length;i++) { var v = Math.abs(parseFloat(table[i][1])||0); var bw = Math.max(Theme.analysis.render.compactBarMinWidth, Math.floor((width-Theme.analysis.render.compactBarWidthPadding) * (v / Math.max(1, maxv)))); ctx.fillStyle = colorForKey((table[i] && table[i].length>0) ? table[i][0] : null, i); ctx.fillRect(Theme.analysis.render.compactBarLeftPadding, i*Theme.analysis.render.compactBarVerticalSpacing + Theme.analysis.render.compactBarTopOffset, bw, Theme.analysis.render.compactBarHeight) }
                return
            }
            for (var i=0;i<table.length;i++) {
                var v = Math.abs(parseFloat(table[i][1])||0)
                var angle = total>0 ? (v/total)*2*Math.PI : (2*Math.PI/table.length)
                var label = (table[i] && table[i].length>0) ? table[i][0] : ""
                var sliceSelected = true
                try { if (legendFilter && legendFilter.length > 0) sliceSelected = (legendFilter.indexOf(label) !== -1) } catch(e) {}
                ctx.beginPath(); ctx.moveTo(cx,cy); ctx.arc(cx,cy,radius,start,start+angle); ctx.closePath(); ctx.globalAlpha = sliceSelected ? 1.0 : 0.25; ctx.fillStyle = colorForKey(label, i); ctx.fill(); ctx.globalAlpha = 1.0; start += angle
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            var x = mouse.x - pieCanvas.width/2
            var y = mouse.y - pieCanvas.height/2
            var a = Math.atan2(y, x)
            var angle = a < -Math.PI/2 ? (a + 2*Math.PI) : a
            var cur = -Math.PI/2
            var total = 0
            for (var i=0;i<table.length;i++) total += Math.abs(parseFloat(table[i][1])||0)
            for (var i=0;i<table.length;i++) {
                var v = Math.abs(parseFloat(table[i][1])||0)
                var ang = total>0 ? (v/total)*2*Math.PI : (2*Math.PI/table.length)
                if (angle >= cur && angle < cur + ang) {
                    var name = (table[i] && table[i].length>0) ? table[i][0] : ""
                    if (!legendFilter) legendFilter = []
                    var idx = legendFilter.indexOf(name)
                    if (idx === -1) legendFilter.push(name); else legendFilter.splice(idx,1)
                    pieCanvas.requestPaint()
                    return
                }
                cur += ang
            }
        }
    }

    function requestPaint() { try { if (pieCanvas) pieCanvas.requestPaint(); } catch(e) {} }
    Component.onCompleted: { try { if (table && table.length>0) requestPaint(); } catch(e) {} }
}
