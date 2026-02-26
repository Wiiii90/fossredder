import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root
    implicitWidth: 600
    implicitHeight: 320
    property var uiData: null
    property var uiDomain: null
    property var table: []
    property var simpleRows: []
    property var contractTypes: []
    property var propertiesList: []
    property var matrix: ({})
    property var _lastConn: null
    Timer { id: initDelay; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { table = (typeof uiData !== 'undefined' && uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []; rebuild() } catch(e) {} } }

    function rebuild() {
        contractTypes = []
        propertiesList = []
        matrix = {}
        try {
            try { console.log('AnalysisTabComponent.rebuild: node uiData present in rootContext?', (typeof Qt !== 'undefined' && Qt.binding) ? true : true) } catch(e) {}
            try { if (typeof uiData !== 'undefined' && uiData && uiData.lastAnalysisResult && uiData.lastAnalysisResult.table && uiData.lastAnalysisResult.table.length>0) { for (var ri=0; ri<Math.min(3, uiData.lastAnalysisResult.table.length); ++ri) { try { console.log('  table['+ri+']=', JSON.stringify(uiData.lastAnalysisResult.table[ri])) } catch(e) {} } } } catch(e) {}
            if (typeof uiData === 'undefined' || !uiData || !uiData.lastAnalysisResult || !uiData.lastAnalysisResult.table) return
            var tbl = uiData.lastAnalysisResult.table
            try { console.log('AnalysisTabComponent.rebuild: sampleRow=', JSON.stringify(tbl[0]), 'tableLen=', tbl.length) } catch(e) {}
            var simple = false
            try {
                if (tbl && tbl.length > 0 && tbl[0].length >= 3) {
                    var testVal = parseFloat(tbl[0][2])
                    if (!isNaN(testVal)) simple = true
                }
            } catch(e) { simple = false }
            try { console.log('AnalysisTabComponent.rebuild: detect simple=', simple, 'type[2]=', typeof(tbl[0][2]), 'val[2]=', tbl[0][2]) } catch(e) {}
            if (simple) {
                simpleRows = JSON.parse(JSON.stringify(tbl))
                contractTypes = []; propertiesList = []; matrix = {}
                table = simpleRows
                try {
                    try { if (typeof matrixContainer !== 'undefined') matrixContainer.visible = false } catch(e) {}
                    try { if (typeof simpleList !== 'undefined') { simpleList.model = simpleRows; simpleList.visible = true } } catch(e) {}
                    updateFlickSizes()
                    console.log('AnalysisTabComponent.rebuild: simpleRows set length=', simpleRows ? simpleRows.length : 0,
                                'innerCol.implicitHeight=', innerCol ? innerCol.implicitHeight : -1,
                                'innerCol.childrenRect.height=', innerCol ? (innerCol.childrenRect ? innerCol.childrenRect.height : -1) : -1,
                                'tabFlick.contentHeight=', tabFlick ? tabFlick.contentHeight : -1,
                                'simpleList.visible=', simpleList ? simpleList.visible : 'n/a',
                                'matrixContainer.visible=', matrixContainer ? matrixContainer.visible : 'n/a')
                } catch(e) { console.log('AnalysisTabComponent.rebuild simpleRows update error', e) }
                return
            } else {
                simpleRows = []
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
        } catch(e) { console.log('AnalysisTabComponent.rebuild error', e) }
    }

    Component.onCompleted: {
        try {
            try { initDelay.start() } catch(e) {}
            console.log('AnalysisTabComponent: component completed, visible=', root.visible, 'width=', root.width, 'height=', root.height)
            try { updateFlickSizes() } catch(e) {}
            try {
                if ((root.width === 0 || root.height === 0) && root.parent) {
                    if (root.parent.width) root.width = root.parent.width
                    if (root.parent.height) root.height = root.parent.height
                    try { root.anchors.fill = root.parent } catch(e) {}
                    console.log('AnalysisTabComponent: adopted parent size width=', root.width, 'height=', root.height)
                }
            } catch(e) { console.log('AnalysisTabComponent: adopt parent size error', e) }
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

    ColumnLayout { anchors.fill: parent; spacing: 6
        Flickable { id: tabFlick; Layout.fillWidth: true; Layout.preferredHeight: 320; clip: true
            Column { id: innerCol; width: tabFlick.width
                Rectangle { id: debugHeader; color: 'transparent'; height: 26; Layout.fillWidth: true; border.color: '#ddd'; border.width: 0
                    RowLayout { anchors.fill: parent; anchors.margins: 4; spacing: 8
                        Label { id: dbgLabel; text: qsTr('Preview: rows=') + (simpleRows ? simpleRows.length : 0) + ' props=' + (propertiesList ? propertiesList.length : 0) + ' contracts=' + (contractTypes ? contractTypes.length : 0); color: '#444' }
                    }
                }

                Item {
                    id: fallbackSimple
                    visible: (table && table.length>0) && (propertiesList.length === 0 || simpleRows.length>0)
                    Layout.fillWidth: true
                    Column { spacing: 4; anchors.fill: parent
                        RowLayout { spacing: 8; height: 28
                            Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; Layout.preferredWidth: 140; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Date') } }
                            Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; Layout.fillWidth: true; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Description') } }
                            Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; Layout.preferredWidth: 100; height: parent.height; Label { anchors.centerIn: parent; text: qsTr('Amount') } }
                        }
                        Repeater { model: (table && table.length>0) ? table : []
                            delegate: RowLayout { spacing: 8; height: 28
                                Rectangle { color: 'transparent'; border.color: '#eee'; border.width: 1; Layout.preferredWidth: 140; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>0) ? modelData[0] : '' } }
                                Rectangle { color: 'transparent'; border.color: '#eee'; border.width: 1; Layout.fillWidth: true; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>1) ? modelData[1] : '' } }
                                Rectangle { color: 'transparent'; border.color: '#eee'; border.width: 1; Layout.preferredWidth: 100; height: parent.height; Label { anchors.centerIn: parent; text: (modelData && modelData.length>2) ? (parseFloat(modelData[2])||0).toFixed(2) : '' } }
                            }
                        }
                    }
                }
                Item { id: matrixContainer; visible: simpleRows.length === 0; Layout.fillWidth: true
                    GridLayout { id: grid; columns: Math.max(2, propertiesList.length + 2); columnSpacing: 8; rowSpacing: 6; Layout.fillWidth: true; Layout.fillHeight: true
                    Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; Layout.preferredWidth: 160; height: 28; Label { anchors.centerIn: parent; text: qsTr('Contract Type'); font.bold: true } }
                    Repeater { model: propertiesList; delegate: Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; height: 28; Layout.preferredWidth: 100; Label { anchors.centerIn: parent; text: modelData; font.bold: true } } }
                    Rectangle { color: 'transparent'; border.color: '#ccc'; border.width: 1; height: 28; Label { anchors.centerIn: parent; text: qsTr('Total'); font.bold: true } }

                    Repeater { model: contractTypes
                        delegate: Column { spacing: 0
                            property string contractName: modelData
                            RowLayout { spacing: 0
                                Rectangle { color: 'transparent'; border.color: '#ddd'; border.width: 1; height: 28; Layout.preferredWidth: 160; Label { anchors.centerIn: parent; text: contractName } }
                                Repeater { model: propertiesList
                                    delegate: Rectangle { color: 'transparent'; border.color: '#eee'; border.width: 1; height: 28; Layout.preferredWidth: 100
                                        Label { anchors.centerIn: parent; text: (function(){ var pid = modelData; var val = 0; try { if (matrix && matrix[contractName] && matrix[contractName][pid]) val = matrix[contractName][pid] } catch(e) { val = 0 } return (typeof val === 'number') ? val.toFixed(2) : String(val) })() }
                                    }
                                }
                                Rectangle { color: 'transparent'; border.color: '#ddd'; border.width: 1; height: 28; Layout.preferredWidth: 100; Label { anchors.centerIn: parent; text: (function(){ var total=0; for (var pi=0; pi<propertiesList.length; ++pi) { var pid = propertiesList[pi]; total += (matrix && matrix[contractName] && matrix[contractName][pid]) ? matrix[contractName][pid] : 0 } return total.toFixed(2) })() } }
                            }
                        }
                    }
                }

                ListView {
                    id: simpleList
                    visible: simpleRows && simpleRows.length > 0
                    model: simpleRows
                    clip: true
                    delegate: RowLayout { spacing: 8; height: 28
                        Label { text: modelData.length>0 ? modelData[0] : ''; Layout.preferredWidth: 140 }
                        Label { text: modelData.length>1 ? modelData[1] : ''; Layout.fillWidth: true }
                        Label { text: (modelData.length>2) ? (parseFloat(modelData[2])||0).toFixed(2) : ''; Layout.preferredWidth: 100; horizontalAlignment: Text.AlignRight }
                    }
                    anchors.left: parent.left; anchors.right: parent.right
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                }
            }
            }
        }
    }

    onUiDataChanged: {
        try {
            table = (uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []
            if (uiData && uiData.lastAnalysisResult) rebuild(); else { contractTypes = []; propertiesList = []; matrix = {} }
        } catch(e) {}
    }

    Connections { target: (typeof uiData !== 'undefined') ? uiData : null
        function onLastAnalysisResultChanged() { try { table = (typeof uiData !== 'undefined' && uiData && uiData.lastAnalysisResult) ? uiData.lastAnalysisResult.table : []; rebuild() } catch(e) {} }
    }
}
