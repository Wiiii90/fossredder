import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: analysisCalcView
    property var uiData
    property var analysisController
    property var calcSelectedTx: []
    width: parent ? parent.width : Theme.analysis.calc.defaultWidth

    function amountForTransaction(transactionId) {
        try {
            var transactions = (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.transactions : []
            for (var i = 0; i < transactions.length; ++i) {
                if (transactions[i] && transactions[i].id === transactionId)
                    return parseFloat(transactions[i].amount)
            }
        } catch (e) {
        }
        return NaN
    }

    function recomputeSelectedAnalysis() {
        try {
            if (!analysisController || !uiData || !uiData.selectedAnalysis)
                return
            var aid = uiData.selectedAnalysis.id
            var filterSpec = uiData.selectedAnalysis.filterSpec ? uiData.selectedAnalysis.filterSpec : ""
            uiData.lastAnalysisResult = analysisController.computeAnalysis(aid, filterSpec)
        } catch (e) {
        }
    }

    function toggleSelectedTransaction(transactionId, checked) {
        if (!transactionId)
            return
        if (checked) {
            if (calcSelectedTx.indexOf(transactionId) === -1)
                calcSelectedTx.push(transactionId)
            return
        }

        var index = calcSelectedTx.indexOf(transactionId)
        if (index !== -1)
            calcSelectedTx.splice(index, 1)
    }

    ColumnLayout { anchors.fill: parent; spacing: Theme.spacingSmall
        RowLayout { spacing: Theme.spacingSmall; Layout.fillWidth: true
            Controls.Button { text: qsTr("New Calc"); onClicked: {
                if (!analysisController) return
                var cfg = JSON.stringify({ strategy: "tax", percent: 0 })
                var id = analysisController.addAnalysis("New Calc", "calc", cfg, "")
                if (id && uiData) uiData.selectedAnalysisId = id
            } }
            Item { Layout.fillWidth: true }
        }

        RowLayout { spacing: Theme.spacingSmall; Layout.fillWidth: true
            Label { text: qsTr("Tax %") }
            Controls.TextField { id: taxPercentField; width: Theme.analysis.calc.taxFieldWidth }
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
                        var amt = amountForTransaction(tx)
                        if (isNaN(amt)) continue
                        var adjusted = amt * (1.0 + percent)
                        obj[tx] = adjusted
                    }
                }
                var j = JSON.stringify(obj)
                if (analysisCalcView.uiData.analyses) analysisCalcView.uiData.analyses.setAdjustmentsById(aid, j)
                recomputeSelectedAnalysis()
            } }
        }

        Label { text: qsTr("Transactions (select to apply tax)") }
        ListView {
            id: calcTxView
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.analysis.calc.transactionListHeight
            model: (analysisCalcView.uiData && analysisCalcView.uiData.lastAnalysisResult && analysisCalcView.uiData.lastAnalysisResult.transactions) ? analysisCalcView.uiData.lastAnalysisResult.transactions : []
            delegate: RowLayout { spacing: Theme.settings.spacing; width: parent.width
                property string txid: (modelData && modelData.id) ? modelData.id : ""
                CheckBox { id: selcb; checked: (analysisCalcView.calcSelectedTx && analysisCalcView.calcSelectedTx.indexOf(txid) !== -1) ? true : false; onCheckedChanged: {
                        toggleSelectedTransaction(txid, checked)
                    } }
                Label { text: (modelData && modelData.date) ? modelData.date : ""; Layout.preferredWidth: Theme.analysis.table.dateColumnWidth }
                Label { text: (modelData && modelData.name) ? modelData.name : ""; Layout.preferredWidth: Theme.analysis.calc.nameColumnWidth }
                Label { text: (modelData && modelData.amount!==undefined && modelData.amount!==null) ? (typeof modelData.amount === 'number' ? modelData.amount.toFixed(2) : String(modelData.amount)) : ""; Layout.preferredWidth: Theme.analysis.calc.taxFieldWidth }
            }
        }
    }
}
