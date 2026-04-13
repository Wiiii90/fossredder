import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    implicitWidth: root.theme.analysis.layout.defaultWidth
    implicitHeight: root.theme.chartPlotPreferredHeight
    property var session: root.appContext ? root.appContext.session : null
    property var analysisController: root.appContext ? root.appContext.analysisController : null
    property var table: []
    property var simpleRows: []
    property var contractTypes: []
    property var propertiesList: []
    property var matrix: ({})
    Timer { id: initDelay; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: { try { root.syncTableFromState(); root.rebuild() } catch(e) {} } }

    function syncTableFromState() {
        root.table = (root.session && root.session.lastAnalysisResult) ? root.session.lastAnalysisResult.table : []
    }

    function resetDerivedData() {
        root.contractTypes = []
        root.propertiesList = []
        root.matrix = {}
        root.simpleRows = []
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
            if (root.matrix && root.matrix[contractName] && root.matrix[contractName][propertyId])
                return root.matrix[contractName][propertyId]
        } catch (e) {
        }
        return 0
    }

    function matrixRowTotal(contractName) {
        let total = 0
        for (let pi = 0; pi < root.propertiesList.length; ++pi)
            total += root.matrixValue(contractName, root.propertiesList[pi])
        return total
    }

    function rebuild() {
        root.resetDerivedData()
        try {
            if (!root.session || !root.session.lastAnalysisResult || !root.session.lastAnalysisResult.table) return
            const tbl = root.session.lastAnalysisResult.table
            if (root.isSimpleTable(tbl)) {
                root.simpleRows = JSON.parse(JSON.stringify(tbl))
                root.table = root.simpleRows
                try { root.updateFlickSizes() } catch(e) { }
                return
            }
            const cset = {}
            const pset = {}
            for (let i=0;i<tbl.length;i++) {
                try {
                    const j = JSON.parse(tbl[i][1])
                    const bc = j.byContract || {}
                    const bp = j.byProperty || {}
                    for (const c in bc) cset[c] = true
                    for (const p in bp) pset[p] = true
                } catch(e) {}
            }
            for (const k in cset) root.contractTypes.push(k)
            for (const k in pset) root.propertiesList.push(k)
            root.contractTypes.sort(); root.propertiesList.sort()
            for (let ci=0; ci<root.contractTypes.length; ++ci) { const ct = root.contractTypes[ci]; root.matrix[ct] = {}; for (let pi=0; pi<root.propertiesList.length; ++pi) root.matrix[ct][root.propertiesList[pi]] = 0 }
            for (let i=0;i<tbl.length;i++) {
                try {
                    const j = JSON.parse(tbl[i][1]); const bc = j.byContract || {}; const bp = j.byProperty || {}
                    let totalContractsVal = 0
                    for (const c in bc) totalContractsVal += parseFloat(bc[c]) || 0
                    for (let pidx=0; pidx<root.propertiesList.length; ++pidx) {
                        const pid = root.propertiesList[pidx]
                        const propVal = parseFloat(bp[pid]) || 0
                        if (bp && bp[pid] && typeof bp[pid] === 'object') {
                            const breakdown = bp[pid]
                            for (const c in breakdown) {
                                const v = parseFloat(breakdown[c]) || 0
                                if (!root.matrix[c]) root.matrix[c] = {}
                                if (!root.matrix[c][pid]) root.matrix[c][pid] = 0
                                root.matrix[c][pid] += v
                            }
                        } else {
                            if (totalContractsVal > 0) {
                                for (const c in bc) {
                                    const cval = parseFloat(bc[c]) || 0
                                    const add = propVal * (cval / totalContractsVal)
                                    if (!root.matrix[c]) root.matrix[c] = {}
                                    if (!root.matrix[c][pid]) root.matrix[c][pid] = 0
                                    root.matrix[c][pid] += add
                                }
                            } else {
                                if (!root.matrix['_ungrouped']) root.matrix['_ungrouped'] = {}
                                if (!root.matrix['_ungrouped'][pid]) root.matrix['_ungrouped'][pid] = 0
                                root.matrix['_ungrouped'][pid] += propVal
                            }
                        }
                    }
                } catch(e) {}
            }
            try { root.updateFlickSizes() } catch(e) {}
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
                let h = 0; let w = 0
                try { h = innerCol.childrenRect ? innerCol.childrenRect.height : (innerCol.implicitHeight || 0) } catch(e) { h = innerCol.implicitHeight || 0 }
                try { w = innerCol.childrenRect ? innerCol.childrenRect.width : (innerCol.implicitWidth || 0) } catch(e) { w = innerCol.implicitWidth || 0 }
                tabFlick.contentHeight = h || 0
                tabFlick.contentWidth = w || 0
            }
        } catch(e) {}
    }

    ColumnLayout { anchors.fill: parent; spacing: root.theme.spacingSmall
        Flickable { id: tabFlick; Layout.fillWidth: true; Layout.preferredHeight: root.theme.chartPlotPreferredHeight; clip: true
            Column { id: innerCol; width: tabFlick.width
                Item {
                    id: fallbackSimple
                    visible: (root.table && root.table.length>0) && (root.propertiesList.length === 0 || root.simpleRows.length>0)
                    Layout.fillWidth: true
                    Column { spacing: root.theme.margins * 2; anchors.fill: parent
                        RowLayout { spacing: root.theme.spacingMedium; height: root.theme.analysis.table.rowHeight
                            Rectangle { color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; Layout.preferredWidth: root.theme.analysis.table.dateColumnWidth; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Date') } }
                            Rectangle { color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; Layout.fillWidth: true; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Description') } }
                            Rectangle { color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Amount') } }
                        }
                        Repeater { model: (root.table && root.table.length>0) ? root.table : []
                            delegate: RowLayout { id: simpleListRow; required property var modelData; spacing: root.theme.spacingMedium; height: root.theme.analysis.table.rowHeight
                                Rectangle { color: 'transparent'; border.color: root.theme.borderLight; border.width: root.theme.borderWidthThin; Layout.preferredWidth: root.theme.analysis.table.dateColumnWidth; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: (simpleListRow.modelData && simpleListRow.modelData.length>0) ? simpleListRow.modelData[0] : '' } }
                                Rectangle { color: 'transparent'; border.color: root.theme.borderLight; border.width: root.theme.borderWidthThin; Layout.fillWidth: true; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: (simpleListRow.modelData && simpleListRow.modelData.length>1) ? simpleListRow.modelData[1] : '' } }
                                Rectangle { color: 'transparent'; border.color: root.theme.borderLight; border.width: root.theme.borderWidthThin; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth; Layout.preferredHeight: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: (simpleListRow.modelData && simpleListRow.modelData.length>2) ? (parseFloat(simpleListRow.modelData[2])||0).toFixed(2) : '' } }
                            }
                        }
                    }
                }
                Item { id: matrixContainer; visible: root.simpleRows.length === 0; Layout.fillWidth: true
                    GridLayout { id: grid; columns: Math.max(2, root.propertiesList.length + 2); columnSpacing: root.theme.spacingMedium; rowSpacing: root.theme.spacingSmall; Layout.fillWidth: true; Layout.fillHeight: true
                    Rectangle { color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; Layout.preferredWidth: root.theme.analysis.table.contractColumnWidth; height: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Contract Type'); font.bold: true } }
                    Repeater { model: root.propertiesList; delegate: Rectangle { id: propertyHeaderCell; required property var modelData; color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; height: root.theme.analysis.table.rowHeight; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth; Label { anchors.centerIn: parent; text: propertyHeaderCell.modelData; font.bold: true } } }
                    Rectangle { color: 'transparent'; border.color: root.theme.borderStrong; border.width: root.theme.borderWidthThin; height: root.theme.analysis.table.rowHeight; Label { anchors.centerIn: parent; text: qsTr('Total'); font.bold: true } }

                    Repeater { model: root.contractTypes
                        delegate: Column { id: contractColumn; required property var modelData; spacing: 0
                            property string contractName: contractColumn.modelData
                            RowLayout { spacing: 0
                                Rectangle { color: 'transparent'; border.color: root.theme.borderMedium; border.width: root.theme.borderWidthThin; height: root.theme.analysis.table.rowHeight; Layout.preferredWidth: root.theme.analysis.table.contractColumnWidth; Label { anchors.centerIn: parent; text: contractColumn.contractName } }
                                Repeater { model: root.propertiesList
                                    delegate: Rectangle { id: matrixValueCell; required property var modelData; color: 'transparent'; border.color: root.theme.borderLight; border.width: root.theme.borderWidthThin; height: root.theme.analysis.table.rowHeight; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth
                                        Label { anchors.centerIn: parent; text: root.matrixValue(contractColumn.contractName, matrixValueCell.modelData).toFixed(2) }
                                    }
                                }
                                Rectangle { color: 'transparent'; border.color: root.theme.borderMedium; border.width: root.theme.borderWidthThin; height: root.theme.analysis.table.rowHeight; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth; Label { anchors.centerIn: parent; text: root.matrixRowTotal(contractColumn.contractName).toFixed(2) } }
                            }
                        }
                    }
                }

                ListView {
                    id: simpleList
                    visible: root.simpleRows && root.simpleRows.length > 0
                    model: root.simpleRows
                    clip: true
                    delegate: RowLayout { id: simpleListRow2; required property var modelData; spacing: root.theme.spacingMedium; height: root.theme.analysis.table.rowHeight
                        Label { text: simpleListRow2.modelData.length>0 ? simpleListRow2.modelData[0] : ''; Layout.preferredWidth: root.theme.analysis.table.dateColumnWidth }
                        Label { text: simpleListRow2.modelData.length>1 ? simpleListRow2.modelData[1] : ''; Layout.fillWidth: true }
                        Label { text: (simpleListRow2.modelData.length>2) ? (parseFloat(simpleListRow2.modelData[2])||0).toFixed(2) : ''; Layout.preferredWidth: root.theme.analysis.table.amountColumnWidth; horizontalAlignment: Text.AlignRight }
                    }
                    anchors.left: parent.left; anchors.right: parent.right
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.analysis.table.detailListHeight
                }
            }
            }
        }
    }

    onSessionChanged: {
        try {
            root.syncTableFromState()
            if (root.session && root.session.lastAnalysisResult) root.rebuild(); else root.resetDerivedData()
        } catch(e) {}
    }

    Connections { target: root.session
        function onLastAnalysisResultChanged() { try { root.syncTableFromState(); root.rebuild() } catch(e) {} }
    }
}
