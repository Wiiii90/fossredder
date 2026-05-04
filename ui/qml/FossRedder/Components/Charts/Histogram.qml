/**
 * @file ui/qml/FossRedder/Components/Charts/Histogram.qml
 * @brief Provides the Histogram component.
 */

import QtQuick 2.15
import FossRedder.Constants 1.0 as Constants
pragma NativeMethodBehavior: AcceptThisObject

Item {
    id: root
    required property var appContext
    required property var theme
    property var table: []
    property var session: root.appContext ? root.appContext.session : null
    property var legendFilter: []
    property real splitProgress: 0.0
    Behavior on splitProgress { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }

    function propertyNameForId(id) {
        if (root.session && root.session.propertyName)
            return root.session.propertyName(id)
        return id
    }

    function colorForKey(k) {
        return Constants.Analysis.colorForKey(k, root.theme.analysis.palette, root.theme.chartFallback)
    }

    Canvas {
        id: histCanvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d"); ctx.reset(); ctx.clearRect(0,0,width,height);
            if (!root.visible || width < root.theme.analysis.layout.minDebugRepaintWidth || !root.table || root.table.length === 0) return;
            const months = [];
            const totals = [];
            const byContract = [];
            const byProperty = [];
            for (let i = 0; i < root.table.length; i++) {
                months.push(root.table[i][0]);
                try { const j = JSON.parse(root.table[i][1]); totals.push(parseFloat(j.total)||0); byContract.push(j.byContract||{}); byProperty.push(j.byProperty||{}) } catch(e) { totals.push(0); byContract.push({}); byProperty.push({}) }
            }
            let maxv = 0; for (let i = 0; i < totals.length; i++) maxv = Math.max(maxv, totals[i]);
            const bw = width / Math.max(1, months.length);
            const categories = {};
            for (let i = 0; i < byContract.length; i++) for (const k in byContract[i]) categories[k]=true
            const catList = Object.keys(categories)

            for (let i = 0; i < months.length; i++) {
                const x = i*bw + root.theme.analysis.render.compactBarTopOffset; let y0 = height - root.theme.analysis.render.histogramBottomPadding
                let baseline = height - root.theme.analysis.render.histogramBottomPadding
                for (let ci = 0; ci < catList.length; ++ci) {
                    const cat = catList[ci]
                    const v = parseFloat(byContract[i][cat]) || 0
                    const h = (maxv>0) ? (v/maxv)*(height-root.theme.analysis.render.histogramTopPadding) : 0
                    const hBase = h * (1 - root.splitProgress)
                    ctx.fillStyle = root.colorForKey(cat)
                    ctx.fillRect(x, baseline - hBase, bw - root.theme.analysis.render.histogramGroupPadding, hBase)
                    baseline -= hBase
                }

                if (root.splitProgress > 0) {
                    const props = []
                    for (const k in byProperty[i]) props.push({k:k,v:byProperty[i][k]})
                    props.sort(function(a,b){ return b.v - a.v })
                    const numProps = Math.max(1, props.length)
                    const gw = Math.max(root.theme.analysis.render.compactBarMinWidth, Math.floor((bw-root.theme.analysis.render.histogramGroupPadding)/numProps))
                    const totalPropsWidth = gw * props.length
                    const startX = x + Math.floor(((bw - root.theme.analysis.render.histogramGroupPadding) - totalPropsWidth) / 2)
                    for (let pi = 0; pi < props.length; ++pi) {
                        let propName = props[pi].k
                        const propVal = parseFloat(props[pi].v) || 0
                        const xprop = startX + pi*gw
                        let y0prop = height - root.theme.analysis.render.histogramBottomPadding
                        let totalContractsVal = 0
                        for (let ci = 0; ci < catList.length; ++ci) totalContractsVal += parseFloat(byContract[i][catList[ci]]) || 0
                        for (let ci = 0; ci < catList.length; ++ci) {
                            const cat = catList[ci]
                            const contractVal = parseFloat(byContract[i][cat]) || 0
                            const v = totalContractsVal > 0 ? (propVal * (contractVal / totalContractsVal)) : 0
                            const h = (maxv>0) ? (v/maxv)*(height-root.theme.analysis.render.histogramTopPadding) : 0
                            const hScaled = h * root.splitProgress
                            ctx.fillStyle = root.colorForKey(cat)
                            ctx.fillRect(xprop, y0prop - hScaled, gw-root.theme.analysis.render.propertyBarInset, hScaled)
                            y0prop -= hScaled
                        }
                        try {
                            ctx.fillStyle = root.theme.chartText
                            ctx.font = root.theme.fontSizeSmall + "px " + root.theme.fontFamily
                            ctx.textBaseline = "top"
                            const label = props[pi].k || ""
                            propName = label
                            try { propName = root.propertyNameForId(label) } catch(e) {}
                            const tw = ctx.measureText ? (ctx.measureText(propName).width || 0) : 0
                            const lx = xprop + ((gw - root.theme.analysis.render.propertyBarInset) / 2) - (tw / 2)
                            if (gw >= root.theme.analysis.render.propertyLabelMinWidth) ctx.fillText(propName, Math.max(xprop, lx), height - (root.theme.spacing * 3))
                        } catch(e) {}
                    ctx.globalAlpha = 1.0
                    }
                    try { if (root.splitProgress !== undefined) {} } catch(e) {}
                }
                try {
                    ctx.fillStyle = root.theme.placeholderText
                    ctx.font = root.theme.fontSizeSmall + "px " + root.theme.fontFamily
                    ctx.textBaseline = "top"
                    const label = months[i]
                    const tw = ctx.measureText ? (ctx.measureText(label).width || 0) : 0
                    const lx = x + ((bw - root.theme.analysis.render.histogramGroupPadding) / 2) - (tw / 2)
                    ctx.fillText(label, Math.max(x, lx), height - (root.theme.spacingMedium + root.theme.margins))
                } catch(e) {}
            }
        }

    }

    function requestPaint() { try { histCanvas.requestPaint(); } catch(e) {} }
    onTableChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }
    Component.onCompleted: { try { if (root.table && root.table.length>0) root.requestPaint(); } catch(e) {} }
    onSplitProgressChanged: { try { if (histCanvas) histCanvas.requestPaint(); } catch(e) {} }

}
