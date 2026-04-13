import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: analysisCalcView
    required property var appContext
    required property var theme
    property var session: analysisCalcView.appContext ? analysisCalcView.appContext.session : null
    property var analysisController: analysisCalcView.appContext ? analysisCalcView.appContext.analysisController : null
    property var calcSelectedTx: []
    width: analysisCalcView.parent ? analysisCalcView.parent.width : analysisCalcView.theme.analysis.calc.defaultWidth

    function recomputeSelectedAnalysis() {
        try {
            if (!analysisCalcView.analysisController || !analysisCalcView.session || !analysisCalcView.session.selectedAnalysis)
                return
            const aid = analysisCalcView.session.selectedAnalysis.id
            const filterSpec = analysisCalcView.session.selectedAnalysis.filterSpec ? analysisCalcView.session.selectedAnalysis.filterSpec : ""
            analysisCalcView.session.lastAnalysisResult = analysisCalcView.analysisController.computeAnalysis(aid, filterSpec)
        } catch (e) {
        }
    }

    function toggleSelectedTransaction(transactionId, checked) {
        if (!transactionId)
            return
        if (checked) {
            if (analysisCalcView.calcSelectedTx.indexOf(transactionId) === -1)
                analysisCalcView.calcSelectedTx.push(transactionId)
            return
        }

        const index = analysisCalcView.calcSelectedTx.indexOf(transactionId)
        if (index !== -1)
            analysisCalcView.calcSelectedTx.splice(index, 1)
    }

    ColumnLayout { anchors.fill: parent; spacing: analysisCalcView.theme.spacingSmall
        RowLayout { spacing: analysisCalcView.theme.spacingSmall; Layout.fillWidth: true
            Controls.Button { text: qsTr("New Calc"); onClicked: {
                if (!analysisCalcView.analysisController) return
                const configJson = analysisCalcView.analysisController.analysisConfigJson("calc", "", "", [], [], 0.0)
                const filterSpec = analysisCalcView.analysisController.analysisFilterSpec("", "")
                const id = analysisCalcView.analysisController.createAnalysis("New Calc", "calc", configJson, filterSpec)
                if (id && analysisCalcView.session) analysisCalcView.session.selectedAnalysisId = id
            } }
            Item { Layout.fillWidth: true }
        }

        RowLayout { spacing: analysisCalcView.theme.spacingSmall; Layout.fillWidth: true
            Label { text: qsTr("Tax %") }
            Controls.TextField { id: taxPercentField; width: analysisCalcView.theme.analysis.calc.taxFieldWidth }
            Controls.Button { text: qsTr("Apply Tax to Selected"); onClicked: {
                if (!analysisCalcView.session || !analysisCalcView.session.selectedAnalysis) return
                const aid = analysisCalcView.session.selectedAnalysis.id
                const filterSpec = analysisCalcView.session.selectedAnalysis.filterSpec ? analysisCalcView.session.selectedAnalysis.filterSpec : ""
                const transactions = analysisCalcView.session.lastAnalysisResult ? analysisCalcView.session.lastAnalysisResult.transactions : []
                let taxPercent = parseFloat(taxPercentField.text)
                if (isNaN(taxPercent)) taxPercent = 0.0
                const adjustmentsJson = analysisCalcView.analysisController ? analysisCalcView.analysisController.analysisAdjustmentsJson(transactions, analysisCalcView.calcSelectedTx, taxPercent) : ""
                if (adjustmentsJson && analysisCalcView.session.analyses) analysisCalcView.session.analyses.setAdjustmentsById(aid, adjustmentsJson)
                const result = analysisCalcView.analysisController ? analysisCalcView.analysisController.computeAnalysis(aid, filterSpec) : {}
                if (result && Object.keys(result).length > 0) analysisCalcView.session.lastAnalysisResult = result
            } }
        }

        Label { text: qsTr("Transactions (select to apply tax)") }
        ListView {
            id: calcTxView
            Layout.fillWidth: true
            Layout.preferredHeight: analysisCalcView.theme.analysis.calc.transactionListHeight
            model: (analysisCalcView.session && analysisCalcView.session.lastAnalysisResult && analysisCalcView.session.lastAnalysisResult.transactions) ? analysisCalcView.session.lastAnalysisResult.transactions : []
            delegate: RowLayout { id: calcTxRow; required property var modelData; spacing: analysisCalcView.theme.settings.spacing; width: calcTxView.width
                property string txid: (calcTxRow.modelData && calcTxRow.modelData.id) ? calcTxRow.modelData.id : ""
                CheckBox { id: selcb; checked: (analysisCalcView.calcSelectedTx && analysisCalcView.calcSelectedTx.indexOf(calcTxRow.txid) !== -1) ? true : false; onCheckedChanged: {
                        analysisCalcView.toggleSelectedTransaction(calcTxRow.txid, checked)
                    } }
                Label { text: (calcTxRow.modelData && calcTxRow.modelData.date) ? calcTxRow.modelData.date : ""; Layout.preferredWidth: analysisCalcView.theme.analysis.table.dateColumnWidth }
                Label { text: (calcTxRow.modelData && calcTxRow.modelData.name) ? calcTxRow.modelData.name : ""; Layout.preferredWidth: analysisCalcView.theme.analysis.calc.nameColumnWidth }
                Label { text: (calcTxRow.modelData && calcTxRow.modelData.amount!==undefined && calcTxRow.modelData.amount!==null) ? (typeof calcTxRow.modelData.amount === 'number' ? calcTxRow.modelData.amount.toFixed(2) : String(calcTxRow.modelData.amount)) : ""; Layout.preferredWidth: analysisCalcView.theme.analysis.calc.taxFieldWidth }
            }
        }
    }
}
