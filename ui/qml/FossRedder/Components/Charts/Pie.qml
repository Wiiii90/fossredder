/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Components/Charts/Pie.qml
 * @brief Provides the Pie component.
 */

import QtQuick 2.15
import FossRedder.Constants 1.0 as Constants

Item {
    id: root
    required property var theme
    property var table: []
    property var legendFilter: []
    property var propertyNameForId
    signal legendFilterChangedByUser(var legendFilter)
    implicitWidth: 320
    implicitHeight: 320
    onTableChanged: { try { if (pieCanvas) pieCanvas.requestPaint(); } catch(e) {} }

    function colorForKey(k, i) {
        try {
            if (k) return Constants.Analysis.colorForKey(k, root.theme.analysis.palette, root.theme.chartFallback)
            return root.theme.analysis.palette[i % root.theme.analysis.palette.length]
        } catch(e) { return root.theme.chartFallback }
    }

    Canvas {
        id: pieCanvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d"); ctx.reset(); ctx.clearRect(0,0,width,height)
            if (!root.visible || width < root.theme.analysis.layout.minDebugRepaintWidth || !root.table || root.table.length === 0) return

            let total = 0
            for (let i = 0; i < root.table.length; i++) total += Math.abs(parseFloat(root.table[i][1]) || 0)
            let start = root.theme.analysis.render.pieStartAngle
            const radius = Math.min(width, height) / 2 - root.theme.analysis.render.pieRadiusPadding
            const cx = width / 2
            const cy = height / 2
            if (radius <= 0) {
                let maxv = 0
                for (let i = 0; i < root.table.length; i++) maxv = Math.max(maxv, Math.abs(parseFloat(root.table[i][1]) || 0))
                for (let i = 0; i < root.table.length; i++) {
                    const v = Math.abs(parseFloat(root.table[i][1]) || 0)
                    const bw = Math.max(root.theme.analysis.render.compactBarMinWidth, Math.floor((width - root.theme.analysis.render.compactBarWidthPadding) * (v / Math.max(1, maxv))))
                    ctx.fillStyle = (root.table[i] && root.table[i].length > 0)
                        ? Constants.Analysis.colorForKey(root.table[i][0], root.theme.analysis.palette, root.theme.chartFallback)
                        : root.theme.analysis.palette[i % root.theme.analysis.palette.length]
                    ctx.fillRect(root.theme.analysis.render.compactBarLeftPadding, i * root.theme.analysis.render.compactBarVerticalSpacing + root.theme.analysis.render.compactBarTopOffset, bw, root.theme.analysis.render.compactBarHeight)
                }
                return
            }
            for (let i = 0; i < root.table.length; i++) {
                const v = Math.abs(parseFloat(root.table[i][1]) || 0)
                const angle = total > 0 ? (v / total) * 2 * Math.PI : (2 * Math.PI / root.table.length)
                const label = (root.table[i] && root.table[i].length > 0) ? root.table[i][0] : ""
                let sliceSelected = true
                try { if (root.legendFilter && root.legendFilter.length > 0) sliceSelected = (root.legendFilter.indexOf(label) !== -1) } catch(e) {}
                ctx.beginPath(); ctx.moveTo(cx,cy); ctx.arc(cx,cy,radius,start,start+angle); ctx.closePath(); ctx.globalAlpha = sliceSelected ? 1.0 : 0.25; ctx.fillStyle = label ? Constants.Analysis.colorForKey(label, root.theme.analysis.palette, root.theme.chartFallback) : root.theme.analysis.palette[i % root.theme.analysis.palette.length]; ctx.fill(); ctx.globalAlpha = 1.0; start += angle
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            const x = mouse.x - pieCanvas.width / 2
            const y = mouse.y - pieCanvas.height / 2
            const a = Math.atan2(y, x)
            const angle = a < -Math.PI / 2 ? (a + 2 * Math.PI) : a
            let cur = -Math.PI / 2
            let total = 0
            for (let i = 0; i < root.table.length; i++) total += Math.abs(parseFloat(root.table[i][1]) || 0)
            for (let i = 0; i < root.table.length; i++) {
                const v = Math.abs(parseFloat(root.table[i][1]) || 0)
                const ang = total > 0 ? (v / total) * 2 * Math.PI : (2 * Math.PI / root.table.length)
                if (angle >= cur && angle < cur + ang) {
                    const name = (root.table[i] && root.table[i].length > 0) ? root.table[i][0] : ""
                    const next = root.legendFilter ? root.legendFilter.slice() : []
                    const idx = next.indexOf(name)
                    if (idx === -1)
                        next.push(name)
                    else
                        next.splice(idx, 1)
                    root.legendFilter = next
                    root.legendFilterChangedByUser(next)
                    pieCanvas.requestPaint()
                    return
                }
                cur += ang
            }
        }
    }

    function requestPaint() { try { if (pieCanvas) pieCanvas.requestPaint(); } catch(e) {} }
    Component.onCompleted: { try { if (root.table && root.table.length > 0) root.requestPaint(); } catch(e) {} }
}
