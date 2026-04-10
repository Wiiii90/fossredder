import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root
    implicitWidth: Theme.analysis.layout.defaultWidth
    implicitHeight: Theme.chartPlotPreferredHeight
    property var session: null
    property var analysisController: null
    property var table: []
    property var simpleRows: []
    property var contractTypes: []
    property var propertiesList: []
    property var matrix: ({})
    Timer { id: initDelay; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { syncTableFromState(); rebuild() } catch(e) {} } }

    function syncTableFromState() {
        table = (typeof session !== 'undefined' && session && session.lastAnalysisResult) ? session.lastAnalysisResult.table : []
    }

    function resetDerivedData() {
        contractTypes = []
        propertiesList = []
        matrix = {}
        simpleRows = []
    }

    function isSimpleTable(tbl) {
        try {
            if (!tbl || tbl.length === 0 || tbl[0].length < 3)
                return false
            return !isNaN(parseFloat(tbl[0][2]))
        } catch (e) {
            return false
        }
    }

    function matrixValue(contractName, propertyId) {
        try {
            if (matrix && matrix[contractName] && matrix[contractName][propertyId])
                return matrix[contractName][propertyId]
        } catch (e) {
        }
        return 0
    }

    function matrixRowTotal(contractName) {
        var total = 0
        for (var pi = 0; pi < propertiesList.length; ++pi)
            total += matrixValue(contractName, propertiesList[pi])
        return total
    }

    function rebuild() {
        resetDerivedData()
        try {
            if (typeof session === 'undefined' || !session || !session.lastAnalysisResult || !session.lastAnalysisResult.table) return
            var tbl = session.lastAnalysisResult.table
            if (isSimpleTable(tbl)) {
                simpleRows = JSON.parse(JSON.stringify(tbl))
                table = simpleRows
                try { updateFlickSizes() } catch(e) { }
                return
            }
            var cset = {}
            var pset = {}
            for (var i=0;i<tbl.length;i++) {
                try {
                    var j = JSON.parse(tbl[i][1])
                    var bc = j.byContract || {}
                    var bp = j.byProperty || {}
                    for (var c in bc) cset[c] = true
                    for (var p in bp) pset[p] = true
                } catch(e) {}
            }
            for (var k in cset) contractTypes.push(k)
            for (var k in pset) propertiesList.push(k)
            contractTypes.sort(); propertiesList.sort()
            for (var ci=0; ci<contractTypes.length; ++ci) { var ct = contractTypes[ci]; matrix[ct] = {}; for (var pi=0; pi<propertiesList.length; ++pi) matrix[ct][propertiesList[pi]] = 0 }
            for (var i=0;i<tbl.length;i++) {
                try {
                    var j = JSON.parse(tbl[i][1]); var bc = j.byContract || {}; var bp = j.byProperty || {}
                    var totalContractsVal = 0
                    for (var c in bc) totalContractsVal += parseFloat(bc[c]) || 0
                    for (var pidx=0; pidx<propertiesList.length; ++pidx) {
                        var pid = propertiesList[pidx]
                        var propVal = parseFloat(bp[pid]) || 0
                        if (bp && bp[pid] && typeof bp[pid] === 'object') {
                            var breakdown = bp[pid]
                            for (var c in breakdown) {
                                var v = parseFloat(breakdown[c]) || 0
                                if (!matrix[c]) matrix[c] = {}
                                if (!matrix[c][pid]) matrix[c][pid] = 0
                                matrix[c][pid] += v
                            }
                        } else {
                            if (totalContractsVal > 0) {
                                for (var c in bc) {
                                    var cval = parseFloat(bc[c]) || 0
                                    var add = propVal * (cval / totalContractsVal)
                                    if (!matrix[c]) matrix[c] = {}
                                    if (!matrix[c][pid]) matrix[c][pid] = 0
                                    matrix[c][pid] += add
                                }
                            } else {
                                if (!matrix['_ungrouped']) matrix['_ungrouped'] = {}
                                if (!matrix['_ungrouped'][pid]) matrix['_ungrouped'][pid] = 0
                                matrix['_ungrouped'][pid] += propVal
                            }
                        }
                    }
                } catch(e) {}
            }
            try { updateFlickSizes() } catch(e) {}
        } catch(e) { }
    }

    Component.onCompleted: {
        try {
            try { initDelay.start() } catch(e) {}
            try { updateFlickSizes() } catch(e) {}
            try {
                if ((root.width === 0 || root.height === 0) && root.parent) {
                    if (root.parent.width) root.width = root.parent.width
                    if (root.parent.height) root.height = root.parent.height
                    try { root.anchors.fill = root.parent } catch(e) {}
                }
            } catch(e) { }
        } catch(e) {}
    }

    function updateFlickSizes() {
        try {
            if (tabFlick && innerCol) {
                var h = 0; var w = 0
                try { h = innerCol.childrenRect ? innerCol.childrenRect.height : (innerCol.implicitHeight || 0) } catch(e) { h = innerCol.implicitHeight || 0 }
                try { w = innerCol.childrenRect ? innerCol.childrenRect.width : (innerCol.implicitWidth || 0) } catch(e) { w = innerCol.implicitWidth || 0 }
                tabFlick.contentHeight = h || 0
                tabFlick.contentWidth = w || 0
            }
        } catch(e) {}
    }

    ColumnLayout { anchors.fill: parent; spacing: Theme.spacingSmall
        Flickable { id: tabFlick; Layout.fillWidth: true; Layout.preferredHeight: Theme.chartPlotPreferredHeight; clip: true
            Column { id: innerCol; width: tabFlick.width
                Item {
                    id: fallbackSimple
                    visible: (table && table.length>0) && (propertiesList.length === 0 || simpleRows.length>0)
                    Layout.fillWidth: true
                    Column { spacing: Theme.margins * 2; anchors.fill: parent
                        RowLayout { spacing: Theme.spacingMedium; height: Theme.analysis.table.rowHeight
                            Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; Layout.preferredWidth: Theme.analysis.table.dateColumnWidth; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Date') } }
                            Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; Layout.fillWidth: true; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Description') } }
                            Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Amount') } }
                        }
                        Repeater { model: (table && table.length>0) ? table : []
                            delegate: RowLayout { spacing: Theme.spacingMedium; height: Theme.analysis.table.rowHeight
                                Rectangle { color: 'transparent'; border.color: Theme.borderLight; border.width: Theme.borderWidthThin; Layout.preferredWidth: Theme.analysis.table.dateColumnWidth; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>0) ? modelData[0] : '' } }
                                Rectangle { color: 'transparent'; border.color: Theme.borderLight; border.width: Theme.borderWidthThin; Layout.fillWidth: true; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>1) ? modelData[1] : '' } }
                                Rectangle { color: 'transparent'; border.color: Theme.borderLight; border.width: Theme.borderWidthThin; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>2) ? (parseFloat(modelData[2])||0).toFixed(2) : '' } }
                            }
                        }
                    }
                }
                Item { id: matrixContainer; visible: simpleRows.length === 0; Layout.fillWidth: true
                    GridLayout { id: grid; columns: Math.max(2, propertiesList.length + 2); columnSpacing: Theme.spacingMedium; rowSpacing: Theme.spacingSmall; Layout.fillWidth: true; Layout.fillHeight: true
                    Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; Layout.preferredWidth: Theme.analysis.table.contractColumnWidth; height: Theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Contract Type'); font.bold: true } }
                    Repeater { model: propertiesList; delegate: Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; height: Theme.analysis.table.rowHeight; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth; Label { anchors.centerIn: parent; text: modelData; font.bold: true } } }
                    Rectangle { color: 'transparent'; border.color: Theme.borderStrong; border.width: Theme.borderWidthThin; height: Theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Total'); font.bold: true } }

                    Repeater { model: contractTypes
                        delegate: Column { spacing: 0
                            property string contractName: modelData
                            RowLayout { spacing: 0
                                Rectangle { color: 'transparent'; border.color: Theme.borderMedium; border.width: Theme.borderWidthThin; height: Theme.analysis.table.rowHeight; Layout.preferredWidth: Theme.analysis.table.contractColumnWidth; Label { anchors.centerIn: parent; text: contractName } }
                                Repeater { model: propertiesList
                                    delegate: Rectangle { color: 'transparent'; border.color: Theme.borderLight; border.width: Theme.borderWidthThin; height: Theme.analysis.table.rowHeight; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth
                                        Label { anchors.centerIn: parent; text: matrixValue(contractName, modelData).toFixed(2) }
                                    }
                                }
                                Rectangle { color: 'transparent'; border.color: Theme.borderMedium; border.width: Theme.borderWidthThin; height: Theme.analysis.table.rowHeight; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth; Label { anchors.centerIn: parent; text: matrixRowTotal(contractName).toFixed(2) } }
                            }
                        }
                    }
                }

                ListView {
                    id: simpleList
                    visible: simpleRows && simpleRows.length > 0
                    model: simpleRows
                    clip: true
                    delegate: RowLayout { spacing: Theme.spacingMedium; height: Theme.analysis.table.rowHeight
                        Label { text: modelData.length>0 ? modelData[0] : ''; Layout.preferredWidth: Theme.analysis.table.dateColumnWidth }
                        Label { text: modelData.length>1 ? modelData[1] : ''; Layout.fillWidth: true }
                        Label { text: (modelData.length>2) ? (parseFloat(modelData[2])||0).toFixed(2) : ''; Layout.preferredWidth: Theme.analysis.table.amountColumnWidth; horizontalAlignment: Text.AlignRight }
                    }
                    anchors.left: parent.left; anchors.right: parent.right
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.analysis.table.detailListHeight
                }
            }
            }
        }
    }

    onSessionChanged: {
        try {
            syncTableFromState()
            if (session && session.lastAnalysisResult) rebuild(); else resetDerivedData()
        } catch(e) {}
    }

    Connections { target: (typeof session !== 'undefined') ? session : null
        function onLastAnalysisResultChanged() { try { syncTableFromState(); rebuild() } catch(e) {} }
    }
}
