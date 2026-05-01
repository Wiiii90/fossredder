import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    property var exportEntries: []

    function annualRows() {
        return root.session ? root.session.annualRows() : []
    }

    function analysisRows() {
        return root.session ? root.session.analysisRows() : []
    }

    function indexForId(rows, id) {
        if (!rows) return -1
        const idText = id ? String(id) : ""
        if (idText.length === 0) return -1
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === idText) return i
        }
        return -1
    }

    function analysisTypeById(id) {
        const rows = analysisRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === id) {
                return rows[i].type ? String(rows[i].type).toLowerCase() : "tab"
            }
        }
        return "tab"
    }

    function exportOptionsForAnalysisType(type) {
        if (String(type).toLowerCase() === "plot") return ["PNG", "JPG"]
        return ["CSV", "XLSX"]
    }

    function defaultExportType(type) {
        const options = exportOptionsForAnalysisType(type)
        return options.length > 0 ? options[0] : "CSV"
    }

    function analysisRowById(id) {
        const rows = analysisRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === id) return rows[i]
        }
        return null
    }

    function annualRowById(id) {
        const rows = annualRows()
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i] && rows[i].id === id) return rows[i]
        }
        return null
    }

    function createAnnualEntry(id, name, analyses) {
        return {
            kind: "annual",
            objectId: id ? id : "",
            objectName: name ? name : "",
            collapsed: false,
            analyses: analyses ? analyses : []
        }
    }

    function createAnalysisEntry(id, name, type, exportType) {
        const typeText = type ? String(type) : ""
        const exportTypeText = exportType ? String(exportType) : ""
        const t = typeText.length > 0 ? typeText.toLowerCase() : analysisTypeById(id)
        const preferredExportType = exportTypeText.length > 0 ? exportTypeText : defaultExportType(t)
        return {
            kind: "analysis",
            objectId: id ? id : "",
            objectName: name ? name : "",
            analysisType: t,
            exportType: preferredExportType
        }
    }

    function analysesForAnnual(annualId, currentAnalyses) {
        const annual = annualRowById(annualId)
        if (!annual || !annual.assignedAnalysisIds || annual.assignedAnalysisIds.length === undefined) return []

        const currentById = {}
        const existing = currentAnalyses ? currentAnalyses : []
        for (let i = 0; i < existing.length; ++i) {
            const current = existing[i]
            if (current && current.objectId) currentById[current.objectId] = current.exportType
        }

        const out = []
        for (let j = 0; j < annual.assignedAnalysisIds.length; ++j) {
            const analysisId = String(annual.assignedAnalysisIds[j])
            const row = analysisRowById(analysisId)
            const type = row && row.type ? String(row.type).toLowerCase() : analysisTypeById(analysisId)
            out.push(createAnalysisEntry(
                analysisId,
                row && row.name ? row.name : "",
                type,
                currentById[analysisId] ? currentById[analysisId] : ""
            ))
        }
        return out
    }

    function addAnnual() {
        const updated = exportEntries.slice()
        updated.push(createAnnualEntry("", "", []))
        exportEntries = updated
    }

    function addStandaloneAnalysis() {
        const updated = exportEntries.slice()
        updated.push(createAnalysisEntry("", "", "tab", "CSV"))
        exportEntries = updated
    }

    function removeEntry(index) {
        if (index < 0 || index >= exportEntries.length) return
        const updated = exportEntries.slice()
        updated.splice(index, 1)
        exportEntries = updated
    }

    function updateAnnual(index, id, name) {
        if (index < 0 || index >= exportEntries.length) return
        const updated = exportEntries.slice()
        const existingAnalyses = updated[index].analyses ? updated[index].analyses : []
        const idText = id ? String(id) : ""
        updated[index].objectId = idText
        updated[index].objectName = name ? name : ""
        updated[index].analyses = idText.length > 0 ? analysesForAnnual(idText, existingAnalyses) : []
        exportEntries = updated
    }

    function updateAnnualCollapsed(index, collapsed) {
        if (index < 0 || index >= exportEntries.length) return
        const updated = exportEntries.slice()
        updated[index].collapsed = !!collapsed
        exportEntries = updated
    }

    function updateStandaloneAnalysis(index, id, name, type, exportType) {
        if (index < 0 || index >= exportEntries.length) return
        const updated = exportEntries.slice()
        const typeText = type ? String(type) : ""
        const exportTypeText = exportType ? String(exportType) : ""
        const t = typeText.length > 0 ? typeText.toLowerCase() : analysisTypeById(id)
        const options = exportOptionsForAnalysisType(t)
        let selectedExportType = exportTypeText.length > 0 ? exportTypeText : updated[index].exportType
        if (options.indexOf(selectedExportType) < 0) selectedExportType = defaultExportType(t)
        updated[index] = createAnalysisEntry(id, name, t, selectedExportType)
        exportEntries = updated
    }

    function updateAnnualAnalysisExportType(entryIndex, analysisIndex, exportType) {
        if (entryIndex < 0 || entryIndex >= exportEntries.length) return
        const updated = exportEntries.slice()
        const analyses = updated[entryIndex].analyses ? updated[entryIndex].analyses.slice() : []
        if (analysisIndex < 0 || analysisIndex >= analyses.length) return
        const analysis = analyses[analysisIndex]
        analyses[analysisIndex] = createAnalysisEntry(
            analysis.objectId,
            analysis.objectName,
            analysis.analysisType,
            exportType)
        updated[entryIndex].analyses = analyses
        exportEntries = updated
    }

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        Label {
            text: qsTr("Export")
            color: root.theme.textPrimary
            font.pointSize: root.theme.fontSizeLarge
            Layout.fillWidth: true
        }

        Controls.Button {
            text: qsTr("Add Annual")
            fillColor: root.theme.surface
            textColor: root.theme.textPrimary
            bordered: true
            onClicked: root.addAnnual()
        }

        Controls.Button {
            text: qsTr("Add Analysis")
            fillColor: root.theme.surface
            textColor: root.theme.textPrimary
            bordered: true
            onClicked: root.addStandaloneAnalysis()
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 180
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border

        Flickable {
            id: objectsFlick
            anchors.fill: parent
            anchors.margins: root.theme.spacing
            clip: true
            contentWidth: width
            contentHeight: Math.max(contentColumn.implicitHeight, objectsFlick.height)

            Column {
                id: contentColumn
                width: objectsFlick.width
                spacing: root.theme.spacing

                Item {
                    width: parent.width
                    height: root.exportEntries.length === 0 ? Math.max(objectsFlick.height, dropContent.implicitHeight) : 0
                    visible: root.exportEntries.length === 0

                    ColumnLayout {
                        id: dropContent
                        anchors.fill: parent
                        anchors.margins: root.theme.spacing
                        spacing: root.theme.spacingSmall

                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            source: Qt.resolvedUrl("../../Assets/export.svg")
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            opacity: 0.85
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Add Annuals here")
                            color: root.theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            font.bold: true
                            wrapMode: Text.WordWrap
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                Repeater {
                    model: root.exportEntries

                    delegate: Loader {
                        required property var modelData
                        required property int index
                        width: contentColumn.width
                        sourceComponent: modelData.kind === "annual" ? annualEntryComponent : standaloneAnalysisComponent
                        onLoaded: {
                            if (!item) return
                            item.entryData = modelData
                            item.entryIndex = index
                            item.width = width
                        }
                    }
                }
            }
        }
    }

    Component {
        id: annualEntryComponent

        Item {
            id: annualEntryItem
            property var entryData: null
            property int entryIndex: -1
            property var annualData: entryData
            width: contentColumn.width
            implicitHeight: annualPanelColumn.implicitHeight

            Column {
                id: annualPanelColumn
                width: parent.width
                spacing: root.theme.spacing

                Rectangle {
                    width: parent.width
                    implicitHeight: annualHeaderRow.implicitHeight + (root.theme.spacing * 2)
                    height: implicitHeight
                    radius: root.theme.radius
                    color: "transparent"
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderSoft

                    RowLayout {
                        id: annualHeaderRow
                        anchors.fill: parent
                        anchors.margins: root.theme.spacing
                        spacing: root.theme.spacing

                        Controls.Button {
                            text: annualEntryItem.annualData.collapsed ? "\u25B6" : "\u25BC"
                            implicitWidth: 28
                            fillColor: "transparent"
                            textColor: root.theme.textMuted
                            onClicked: root.updateAnnualCollapsed(annualEntryItem.entryIndex, !annualEntryItem.annualData.collapsed)
                        }

                        Label {
                            text: qsTr("Annual")
                            color: root.theme.textPrimary
                            Layout.preferredWidth: 70
                        }

                        Controls.ComboBox {
                            Layout.fillWidth: true
                            model: root.annualRows()
                            textRole: "display"
                            currentIndex: root.indexForId(root.annualRows(), annualEntryItem.annualData.objectId)
                            onActivated: {
                                const row = model && currentIndex >= 0 ? model[currentIndex] : null
                                root.updateAnnual(annualEntryItem.entryIndex,
                                                  row && row.id ? row.id : "",
                                                  row && row.name ? row.name : "")
                            }
                        }

                        Controls.Button {
                            text: "×"
                            implicitWidth: 28
                            fillColor: root.theme.surface
                            textColor: root.theme.textMuted
                            bordered: true
                            onClicked: root.removeEntry(annualEntryItem.entryIndex)
                        }
                    }
                }

                Column {
                    width: parent.width
                    leftPadding: root.theme.spacing + root.theme.margins
                    rightPadding: root.theme.spacing
                    spacing: root.theme.spacing
                    visible: !annualEntryItem.annualData.collapsed

                    Label {
                        width: parent.width
                        visible: (!annualEntryItem.annualData.analyses || annualEntryItem.annualData.analyses.length === 0)
                        text: qsTr("No analyses assigned")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                    }

                    Repeater {
                        model: annualEntryItem.annualData.analyses ? annualEntryItem.annualData.analyses : []

                        delegate: Rectangle {
                            id: annualAnalysisEntry
                            required property var modelData
                            required property int index
                            width: parent.width
                            implicitHeight: annualAnalysisRow.implicitHeight + (root.theme.spacing * 2)
                            height: implicitHeight
                            radius: root.theme.radius
                            color: "transparent"
                            border.color: root.theme.borderSoft
                            border.width: root.theme.borderWidthThin

                            RowLayout {
                                id: annualAnalysisRow
                                anchors.fill: parent
                                anchors.margins: root.theme.spacing
                                spacing: root.theme.spacing

                                Item { Layout.preferredWidth: 28 }

                                Label {
                                    text: qsTr("Analysis")
                                    color: root.theme.textPrimary
                                    Layout.preferredWidth: 70
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: annualAnalysisEntry.modelData.objectName && annualAnalysisEntry.modelData.objectName.length > 0
                                          ? annualAnalysisEntry.modelData.objectName
                                          : qsTr("(unassigned)")
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                }

                                Controls.ComboBox {
                                    Layout.preferredWidth: 110
                                    model: root.exportOptionsForAnalysisType(annualAnalysisEntry.modelData.analysisType)
                                    currentIndex: Math.max(0, model.indexOf(annualAnalysisEntry.modelData.exportType))
                                    onActivated: {
                                        const selectedExportType = model && currentIndex >= 0 ? model[currentIndex] : ""
                                        root.updateAnnualAnalysisExportType(annualEntryItem.entryIndex, annualAnalysisEntry.index, selectedExportType)
                                    }
                                }

                                Item { Layout.preferredWidth: 28 }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: standaloneAnalysisComponent

        Rectangle {
            id: standaloneAnalysisEntry
            property var entryData: null
            property int entryIndex: -1
            property var modelData: entryData
            property int index: entryIndex
            width: contentColumn.width
            implicitHeight: standaloneAnalysisRow.implicitHeight + (root.theme.spacing * 2)
            height: implicitHeight
            radius: root.theme.radius
            color: "transparent"
            border.color: root.theme.borderSoft
            border.width: root.theme.borderWidthThin

            RowLayout {
                id: standaloneAnalysisRow
                anchors.fill: parent
                anchors.margins: root.theme.spacing
                spacing: root.theme.spacing

                Label {
                    text: qsTr("Analysis")
                    color: root.theme.textPrimary
                    Layout.preferredWidth: 70
                }

                Controls.ComboBox {
                    Layout.fillWidth: true
                    model: root.analysisRows()
                    textRole: "name"
                    currentIndex: root.indexForId(root.analysisRows(), standaloneAnalysisEntry.modelData.objectId)
                    onActivated: {
                        const row = model && currentIndex >= 0 ? model[currentIndex] : null
                        const type = row && row.type ? String(row.type).toLowerCase() : "tab"
                        root.updateStandaloneAnalysis(standaloneAnalysisEntry.entryIndex,
                                                      row && row.id ? row.id : "",
                                                      row && row.name ? row.name : "",
                                                      type,
                                                      "")
                    }
                }

                Controls.ComboBox {
                    Layout.preferredWidth: 110
                    model: root.exportOptionsForAnalysisType(standaloneAnalysisEntry.modelData.analysisType)
                    currentIndex: Math.max(0, model.indexOf(standaloneAnalysisEntry.modelData.exportType))
                    onActivated: {
                        const selectedExportType = model && currentIndex >= 0 ? model[currentIndex] : ""
                        root.updateStandaloneAnalysis(standaloneAnalysisEntry.entryIndex,
                                                      standaloneAnalysisEntry.modelData.objectId,
                                                      standaloneAnalysisEntry.modelData.objectName,
                                                      standaloneAnalysisEntry.modelData.analysisType,
                                                      selectedExportType)
                    }
                }

                Controls.Button {
                    text: "×"
                    implicitWidth: 28
                    fillColor: root.theme.surface
                    textColor: root.theme.textMuted
                    bordered: true
                    onClicked: root.removeEntry(standaloneAnalysisEntry.entryIndex)
                }
            }
        }
    }

    function exportItems() {
        const out = []
        for (let i = 0; i < exportEntries.length; ++i) {
            const entry = exportEntries[i]
            if (entry.kind === "annual") {
                out.push({
                    objectType: "Annual",
                    objectId: entry.objectId ? entry.objectId : "",
                    objectName: entry.objectName ? entry.objectName : "",
                    exportType: ""
                })
                const analyses = entry.analyses ? entry.analyses : []
                for (let j = 0; j < analyses.length; ++j) {
                    const analysis = analyses[j]
                    out.push({
                        objectType: "Analysis",
                        annualId: entry.objectId ? entry.objectId : "",
                        objectId: analysis.objectId ? analysis.objectId : "",
                        objectName: analysis.objectName ? analysis.objectName : "",
                        exportType: analysis.exportType ? analysis.exportType : ""
                    })
                }
                continue
            }
            out.push({
                objectType: "Analysis",
                annualId: "",
                objectId: entry.objectId ? entry.objectId : "",
                objectName: entry.objectName ? entry.objectName : "",
                exportType: entry.exportType ? entry.exportType : ""
            })
        }
        return out
    }

    function clearAll() {
        exportEntries = []
    }

    function loadItems(items) {
        exportEntries = []
        if (!items) return

        const loadedEntries = []
        for (let i = 0; i < items.length; ++i) {
            const item = items[i]
            if (!item) continue

            if (item.objectType === "Annual") {
                const annualId = item.objectId ? item.objectId : ""
                const annualName = item.objectName ? item.objectName : ""
                loadedEntries.push(createAnnualEntry(annualId, annualName, analysesForAnnual(annualId, [])))
                continue
            }

            if (item.objectType === "Analysis") {
                const annualId = item.annualId ? item.annualId : ""
                let mappedToAnnual = false
                if (annualId && annualId.length > 0) {
                    for (let k = loadedEntries.length - 1; k >= 0; --k) {
                        const entry = loadedEntries[k]
                        if (entry.kind === "annual" && entry.objectId === annualId) {
                            const analyses = entry.analyses ? entry.analyses.slice() : []
                            const type = analysisTypeById(item.objectId ? item.objectId : "")
                            const analysisEntry = createAnalysisEntry(item.objectId, item.objectName, type, item.exportType)
                            analyses.push(analysisEntry)
                            entry.analyses = analyses
                            loadedEntries[k] = entry
                            mappedToAnnual = true
                            break
                        }
                    }
                }

                if (!mappedToAnnual) {
                    const row = analysisRowById(item.objectId ? item.objectId : "")
                    const type = row && row.type ? String(row.type).toLowerCase() : analysisTypeById(item.objectId ? item.objectId : "")
                    loadedEntries.push(createAnalysisEntry(item.objectId, item.objectName, type, item.exportType))
                }
            }
        }

        exportEntries = loadedEntries
    }
}
