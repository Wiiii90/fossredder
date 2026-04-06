import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: analysisCalcView
    property var session
    property var analysisController
    property var calcSelectedTx: []
    width: parent ? parent.width : Theme.analysis.calc.defaultWidth

    function recomputeSelectedAnalysis() {
        try {
            if (!analysisController || !session || !session.selectedAnalysis)
                return
            var aid = session.selectedAnalysis.id
            var filterSpec = session.selectedAnalysis.filterSpec ? session.selectedAnalysis.filterSpec : ""
            session.lastAnalysisResult = analysisController.computeAnalysis(aid, filterSpec)
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
                if (id && session) session.selectedAnalysisId = id
            } }
            Item { Layout.fillWidth: true }
        }

        RowLayout { spacing: Theme.spacingSmall; Layout.fillWidth: true
            Label { text: qsTr("Tax %") }
            Controls.TextField { id: taxPercentField; width: Theme.analysis.calc.taxFieldWidth }
            Controls.Button { text: qsTr("Apply Tax to Selected"); onClicked: {
                if (!analysisCalcView.session || !analysisCalcView.session.selectedAnalysis) return
                var aid = analysisCalcView.session.selectedAnalysis.id
                var filterSpec = analysisCalcView.session.selectedAnalysis.filterSpec ? analysisCalcView.session.selectedAnalysis.filterSpec : ""
                var transactions = analysisCalcView.session.lastAnalysisResult ? analysisCalcView.session.lastAnalysisResult.transactions : []
                var result = analysisController ? analysisController.applyTaxAdjustmentsAndRecomputeFromText(aid, filterSpec, transactions, analysisCalcView.calcSelectedTx, taxPercentField.text) : {}
                if (result && result.adjustmentsJson && analysisCalcView.session.analyses) analysisCalcView.session.analyses.setAdjustmentsById(aid, result.adjustmentsJson)
                if (result && result.analysisResult) analysisCalcView.session.lastAnalysisResult = result.analysisResult
            } }
        }

        Label { text: qsTr("Transactions (select to apply tax)") }
        ListView {
            id: calcTxView
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.analysis.calc.transactionListHeight
            model: (analysisCalcView.session && analysisCalcView.session.lastAnalysisResult && analysisCalcView.session.lastAnalysisResult.transactions) ? analysisCalcView.session.lastAnalysisResult.transactions : []
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
