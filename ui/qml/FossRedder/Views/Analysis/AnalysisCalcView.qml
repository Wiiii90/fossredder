import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: analysisCalcView
    property var uiData
    property var analysisController
    width: parent ? parent.width : 400

    ColumnLayout { anchors.fill: parent; spacing: 6
        RowLayout { spacing: 6; Layout.fillWidth: true
            Controls.Button { text: qsTr("New Calc"); onClicked: {
                if (!analysisController) return
                var cfg = JSON.stringify({ strategy: "tax", percent: 0 })
                var id = analysisController.addAnalysis("New Calc", "calc", cfg, "")
                if (id && uiData) uiData.selectedAnalysisId = id
            } }
            Item { Layout.fillWidth: true }
        }

        RowLayout { spacing: 6; Layout.fillWidth: true
            Label { text: qsTr("Tax %") }
            Controls.TextField { id: taxPercentField; width: 80 }
            Controls.Button { text: qsTr("Apply Tax to Selected"); onClicked: {
                if (!analysisCalcView.uiData || !analysisCalcView.uiData.selectedAnalysis) return
                var aid = analysisCalcView.uiData.selectedAnalysis.id
                var obj = {}
                var percent = parseFloat(taxPercentField.text)
                if (isNaN(percent)) percent = 0.0
                if (analysisCalcView.calcSelectedTx && analysisCalcView.calcSelectedTx.length > 0) {
                    for (var i = 0; i < analysisCalcView.calcSelectedTx.length; ++i) {
                        var tx = analysisCalcView.calcSelectedTx[i]
                        if (!tx) continue
                        var amt = null
                        try {
                            var tbl = analysisCalcView.uiData.lastAnalysisResult.transactions
                            for (var j = 0; j < tbl.length; ++j) {
                                if (tbl[j] && tbl[j].id === tx) { amt = parseFloat(tbl[j].amount); break }
                            }
                        } catch(e) {}
                        if (amt === null || isNaN(amt)) continue
                        var adjusted = amt * (1.0 + percent)
                        obj[tx] = adjusted
                    }
                }
                var j = JSON.stringify(obj)
                if (analysisCalcView.uiData.analyses) analysisCalcView.uiData.analyses.setAdjustmentsById(aid, j)
                try { var res = analysisCalcView.analysisController.computeAnalysis(aid, analysisCalcView.uiData.selectedAnalysis.filterSpec ? analysisCalcView.uiData.selectedAnalysis.filterSpec : ""); analysisCalcView.uiData.lastAnalysisResult = res; } catch(e) {}
            } }
        }

        Label { text: qsTr("Transactions (select to apply tax)") }
        ListView {
            id: calcTxView
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            model: (analysisCalcView.uiData && analysisCalcView.uiData.lastAnalysisResult && analysisCalcView.uiData.lastAnalysisResult.transactions) ? analysisCalcView.uiData.lastAnalysisResult.transactions : []
            delegate: RowLayout { spacing: 8; width: parent.width
                property string txid: (modelData && modelData.id) ? modelData.id : ""
                CheckBox { id: selcb; checked: (analysisCalcView.calcSelectedTx && analysisCalcView.calcSelectedTx.indexOf(txid) !== -1) ? true : false; onCheckedChanged: {
                        if (!analysisCalcView.calcSelectedTx) analysisCalcView.calcSelectedTx = []
                        if (checked) { if (analysisCalcView.calcSelectedTx.indexOf(txid) === -1) analysisCalcView.calcSelectedTx.push(txid) }
                        else { var idx = analysisCalcView.calcSelectedTx.indexOf(txid); if (idx !== -1) analysisCalcView.calcSelectedTx.splice(idx,1) }
                    } }
                Label { text: (modelData && modelData.date) ? modelData.date : ""; Layout.preferredWidth: 120 }
                Label { text: (modelData && modelData.name) ? modelData.name : ""; Layout.preferredWidth: 200 }
                Label { text: (modelData && modelData.amount!==undefined && modelData.amount!==null) ? (typeof modelData.amount === 'number' ? modelData.amount.toFixed(2) : String(modelData.amount)) : ""; Layout.preferredWidth: 80 }
            }
        }
    }
}
