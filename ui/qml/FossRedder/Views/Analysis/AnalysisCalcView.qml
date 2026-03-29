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
                var id = analysisController.createAnalysisFromUi("New Calc", "calc", "", "", [], [], "", "", 0.0)
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
                var filterSpec = analysisCalcView.uiData.selectedAnalysis.filterSpec ? analysisCalcView.uiData.selectedAnalysis.filterSpec : ""
                var percent = parseFloat(taxPercentField.text)
                if (isNaN(percent)) percent = 0.0
                var transactions = analysisCalcView.uiData.lastAnalysisResult ? analysisCalcView.uiData.lastAnalysisResult.transactions : []
                var result = analysisController ? analysisController.applyTaxAdjustmentsAndRecompute(aid, filterSpec, transactions, analysisCalcView.calcSelectedTx, percent) : {}
                if (result && result.adjustmentsJson && analysisCalcView.uiData.analyses) analysisCalcView.uiData.analyses.setAdjustmentsById(aid, result.adjustmentsJson)
                if (result && result.analysisResult) analysisCalcView.uiData.lastAnalysisResult = result.analysisResult
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
