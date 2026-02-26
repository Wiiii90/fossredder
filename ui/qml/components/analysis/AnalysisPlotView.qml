import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root
    property var uiData
    property var uiDomain

    function hashString(s) { var h = 0; if (!s) return 0; for (var i=0;i<s.length;i++) { h = ((h<<5) - h) + s.charCodeAt(i); h |= 0 } return Math.abs(h) }
    function colorForKey(k) { try { var palette = ["#8dd3c7","#ffffb3","#bebada","#fb8072","#80b1d3","#fdb462","#b3de69","#fccde5","#d9d9d9","#bc80bd","#ccebc5","#ffed6f"]; return palette[hashString(k) % palette.length] } catch(e) { return "#888" } }

    ColumnLayout { anchors.fill: parent; spacing: 6
        AppPie { id: pie; Layout.fillWidth: true; Layout.preferredHeight: 320; visible: false }
        AppHistogram { id: hist; Layout.fillWidth: true; Layout.preferredHeight: 320; visible: false }

        RowLayout { Layout.fillWidth: true; spacing: 8
            Item { Layout.fillWidth: true }
            RowLayout { spacing: 6; visible: false
                Label { text: qsTr('Split by property') }
                Switch { id: splitSwitch; onCheckedChanged: { hist.splitProgress = checked ? 1.0 : 0.0 } }
            }
        }

        Label { text: qsTr('Legend') }
        Flickable { id: legendFlick; Layout.fillWidth: true; Layout.preferredHeight: 120; clip: true }
        Column { id: histLegendList; width: parent.width }
    }

    function rebuild() {
        try {
            if (!uiData || !uiData.lastAnalysisResult) { pie.visible = false; hist.visible = false; return }
            var t = uiData.lastAnalysisResult.type ? uiData.lastAnalysisResult.type : (uiData.lastAnalysisResult.config ? (function(){ try { var c = JSON.parse(uiData.lastAnalysisResult.config); return c.plotType } catch(e){ return "" } })() : "")
            pie.visible = (t === "pie")
            hist.visible = (t === "histogram")
            if (pie.visible) { pie.uiData = uiData; pie.table = uiData.lastAnalysisResult.table }
            if (hist.visible) { hist.uiData = uiData; hist.table = uiData.lastAnalysisResult.table }
        } catch(e) { console.log('AnalysisPlotView.rebuild error', e) }
    }

    Component.onCompleted: rebuild()
    Connections { target: uiData; onLastAnalysisResultChanged: rebuild }
}
