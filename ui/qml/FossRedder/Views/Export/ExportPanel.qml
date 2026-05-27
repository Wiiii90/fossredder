/**
 * @file ui/qml/FossRedder/Views/Export/ExportPanel.qml
 * @brief Provides the ExportPanel component.
 */

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
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0

    property var exportEntries: []
    property string addMode: "annual"
    property string pendingAnnualId: ""
    property string pendingAnalysisId: ""
    property int exportTypeColumnWidth: 110
    property int removeColumnWidth: root.theme.viewCompactActionButtonSize
    property int leadingColumnWidth: root.theme.viewCompactActionButtonSize
    property int kindColumnWidth: 88
    property int analysisNameMinWidth: 160

    function annualRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.annualRows() : []
    }

    function analysisRows() {
        const _workspaceRevision = root.workspaceRevision
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

    function normalizeExportType(exportType, type) {
        const options = exportOptionsForAnalysisType(type)
        const exportTypeText = exportType ? String(exportType).toUpperCase() : ""
        return options.indexOf(exportTypeText) >= 0 ? exportTypeText : defaultExportType(type)
    }

    function defaultExportType(type) {
        const options = exportOptionsForAnalysisType(type)
        return options.length > 0 ? options[0] : "CSV"
    }

    function addRows() {
        return root.addMode === "annual" ? root.annualRows() : root.analysisRows()
    }

    function addTextRole() {
        return root.addMode === "annual" ? "display" : "name"
    }

    function pendingObjectId() {
        return root.addMode === "annual" ? root.pendingAnnualId : root.pendingAnalysisId
    }

    function pendingIndex() {
        return root.indexForId(root.addRows(), root.pendingObjectId())
    }

    function ensurePendingSelection() {
        const rows = root.addRows()
        if (!rows || rows.length === 0) {
            if (root.addMode === "annual")
                root.pendingAnnualId = ""
            else
                root.pendingAnalysisId = ""
            return
        }

        const currentId = root.pendingObjectId()
        if (root.indexForId(rows, currentId) >= 0)
            return

        const firstRow = rows[0]
        const firstId = firstRow && firstRow.id ? String(firstRow.id) : ""
        if (root.addMode === "annual")
            root.pendingAnnualId = firstId
        else
            root.pendingAnalysisId = firstId
    }

    function syncWithWorkspaceRevision() {
        const _workspaceRevision = root.workspaceRevision
        root.ensurePendingSelection()
    }

    function selectPendingRow(index) {
        const rows = root.addRows()
        const row = rows && index >= 0 && index < rows.length ? rows[index] : null
        const nextId = row && row.id ? String(row.id) : ""
        if (root.addMode === "annual")
            root.pendingAnnualId = nextId
        else
            root.pendingAnalysisId = nextId
    }

    function addPendingEntry() {
        const objectId = root.pendingObjectId()
        if (!objectId || objectId.length === 0)
            return

        const updated = exportEntries.slice()
        if (root.addMode === "annual") {
            const annual = root.annualRowById(objectId)
            updated.push(createAnnualEntry(objectId,
                                           annual && annual.name ? annual.name : "",
                                           analysesForAnnual(objectId, [])))
        } else {
            const analysis = root.analysisRowById(objectId)
            const analysisType = analysis && analysis.type ? String(analysis.type).toLowerCase() : analysisTypeById(objectId)
            updated.push(createAnalysisEntry(objectId,
                                             analysis && analysis.name ? analysis.name : "",
                                             analysisType,
                                             analysis && analysis.exportFormat ? analysis.exportFormat : ""))
        }
        exportEntries = updated
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
        const t = typeText.length > 0 ? typeText.toLowerCase() : analysisTypeById(id)
        const preferredExportType = normalizeExportType(exportType, t)
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
        if (!annual) return []
        const annualAnalysisIds = annual.analysisIds ? annual.analysisIds : annual.assignedAnalysisIds
        if (!annualAnalysisIds || annualAnalysisIds.length === undefined) return []

        const currentById = {}
        const existing = currentAnalyses ? currentAnalyses : []
        for (let i = 0; i < existing.length; ++i) {
            const current = existing[i]
            if (current && current.objectId) currentById[current.objectId] = current.exportType
        }

        const out = []
        for (let j = 0; j < annualAnalysisIds.length; ++j) {
            const analysisId = String(annualAnalysisIds[j])
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
        root.addMode = "annual"
        root.ensurePendingSelection()
        root.addPendingEntry()
    }

    function addStandaloneAnalysis() {
        root.addMode = "analysis"
        root.ensurePendingSelection()
        root.addPendingEntry()
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
        const t = typeText.length > 0 ? typeText.toLowerCase() : analysisTypeById(id)
        const options = exportOptionsForAnalysisType(t)
        let selectedExportType = exportType ? String(exportType).toUpperCase() : updated[index].exportType
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

    onAddModeChanged: root.ensurePendingSelection()

    Connections {
        target: root.session
        function onDataRevisionChanged() {
            root.syncWithWorkspaceRevision()
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Controls.Button {
            objectName: "exportAddAnnualModeButton"
            text: qsTr("Annual")
            bordered: true
            filled: root.addMode === "annual"
            fillColor: root.addMode === "annual" ? root.theme.subtlePrimaryFill : root.theme.surface
            textColor: root.addMode === "annual" ? root.theme.textPrimary : root.theme.textPrimary
            Layout.preferredWidth: 88
            Layout.preferredHeight: root.theme.controlHeight
            onClicked: root.addMode = "annual"
        }

        Controls.Button {
            objectName: "exportAddAnalysisModeButton"
            text: qsTr("Analysis")
            bordered: true
            filled: root.addMode === "analysis"
            fillColor: root.addMode === "analysis" ? root.theme.subtlePrimaryFill : root.theme.surface
            textColor: root.addMode === "analysis" ? root.theme.textPrimary : root.theme.textPrimary
            Layout.preferredWidth: 88
            Layout.preferredHeight: root.theme.controlHeight
            onClicked: root.addMode = "analysis"
        }

        Controls.DropdownMenu {
            id: addObjectDropdown
            objectName: "exportAddObjectComboBox"
            Layout.fillWidth: true
            Layout.preferredWidth: root.theme.formFieldWidth
            model: root.addRows()
            textRole: root.addTextRole()
            currentIndex: root.pendingIndex()
            onActivated: function(index) { root.selectPendingRow(index) }
        }

        Controls.AddButton {
            objectName: "exportAddEntryButton"
            Layout.preferredWidth: 72
            Layout.minimumWidth: 72
            Layout.maximumWidth: 72
            Layout.preferredHeight: root.theme.controlHeight
            Layout.minimumHeight: root.theme.controlHeight
            Layout.maximumHeight: root.theme.controlHeight
            enabled: root.pendingObjectId().length > 0
            onClicked: root.addPendingEntry()
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
                            Layout.preferredWidth: root.theme.viewSectionIconSize
                            Layout.preferredHeight: root.theme.viewSectionIconSize
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
                    implicitHeight: annualCardColumn.implicitHeight + (root.theme.spacing * 2)
                    height: implicitHeight
                    radius: root.theme.radius
                    color: "transparent"
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderSoft

                    Column {
                        id: annualCardColumn
                        anchors.fill: parent
                        anchors.margins: root.theme.spacing
                        spacing: root.theme.spacing

                        RowLayout {
                            id: annualHeaderRow
                            width: parent.width
                            spacing: root.theme.spacing

                            Controls.SecondaryButton {
                                text: annualEntryItem.annualData.collapsed ? "\u25B6" : "\u25BC"
                                Layout.preferredWidth: root.leadingColumnWidth
                                Layout.minimumWidth: root.leadingColumnWidth
                                Layout.maximumWidth: root.leadingColumnWidth
                                Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                textColor: root.theme.textMuted
                                onClicked: root.updateAnnualCollapsed(annualEntryItem.entryIndex, !annualEntryItem.annualData.collapsed)
                            }

                            Rectangle {
                                Layout.preferredWidth: root.kindColumnWidth
                                Layout.preferredHeight: root.theme.controlHeight
                                radius: root.theme.radius
                                color: root.theme.surface
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.borderSoft

                                Label {
                                    anchors.fill: parent
                                    text: qsTr("Annual")
                                    color: root.theme.textPrimary
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                            }

                            Controls.DropdownMenu {
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

                            Item {
                                Layout.preferredWidth: root.exportTypeColumnWidth
                                Layout.minimumWidth: root.exportTypeColumnWidth
                                Layout.maximumWidth: root.exportTypeColumnWidth
                            }

                            Controls.SecondaryButton {
                                text: "×"
                                Layout.preferredWidth: root.removeColumnWidth
                                Layout.minimumWidth: root.removeColumnWidth
                                Layout.maximumWidth: root.removeColumnWidth
                                Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                textColor: root.theme.textMuted
                                onClicked: root.removeEntry(annualEntryItem.entryIndex)
                            }
                        }
                        Column {
                            id: annualAnalysesColumn
                            width: parent.width
                            spacing: root.theme.spacing
                            visible: !annualEntryItem.annualData.collapsed

                            Label {
                                width: parent.width
                                visible: (!annualEntryItem.annualData.analyses || annualEntryItem.annualData.analyses.length === 0)
                                text: qsTr("No analyses assigned")
                                color: root.theme.textMuted
                                wrapMode: Text.WordWrap
                                leftPadding: root.leadingColumnWidth + root.kindColumnWidth + root.theme.spacing * 2
                            }

                            Repeater {
                                model: annualEntryItem.annualData.analyses ? annualEntryItem.annualData.analyses : []

                                delegate: Rectangle {
                                    id: annualAnalysisEntry
                                    required property var modelData
                                    required property int index
                                    width: annualAnalysesColumn.width
                                    implicitHeight: annualAnalysisRow.implicitHeight + (root.theme.spacing * 2)
                                    height: implicitHeight
                                    radius: root.theme.radius
                                    color: "transparent"
                                    border.color: root.theme.borderSoft
                                    border.width: root.theme.borderWidthThin

                                    RowLayout {
                                        id: annualAnalysisRow
                                        anchors.fill: parent
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.topMargin: root.theme.spacing
                                        anchors.bottomMargin: root.theme.spacing
                                        spacing: root.theme.spacing

                                        Item {
                                            Layout.preferredWidth: root.leadingColumnWidth
                                            Layout.minimumWidth: root.leadingColumnWidth
                                            Layout.maximumWidth: root.leadingColumnWidth
                                        }

                                        Rectangle {
                                            Layout.preferredWidth: root.kindColumnWidth
                                            Layout.preferredHeight: root.theme.controlHeight
                                            radius: root.theme.radius
                                            color: root.theme.surface
                                            border.width: root.theme.borderWidthThin
                                            border.color: root.theme.borderSoft

                                            Label {
                                                anchors.fill: parent
                                                text: qsTr("Analysis")
                                                color: root.theme.textPrimary
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                font.bold: true
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: root.analysisNameMinWidth
                                            Layout.preferredHeight: root.theme.controlHeight
                                            radius: root.theme.radius
                                            color: root.theme.surface
                                            border.width: root.theme.borderWidthThin
                                            border.color: root.theme.borderSoft

                                            Label {
                                                anchors.fill: parent
                                                anchors.leftMargin: root.theme.spacing
                                                anchors.rightMargin: root.theme.spacing
                                                text: annualAnalysisEntry.modelData.objectName && annualAnalysisEntry.modelData.objectName.length > 0
                                                      ? annualAnalysisEntry.modelData.objectName
                                                      : qsTr("(unassigned)")
                                                color: root.theme.textPrimary
                                                horizontalAlignment: Text.AlignLeft
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                            }
                                        }

                                        Controls.DropdownMenu {
                                            Layout.preferredWidth: root.exportTypeColumnWidth
                                            Layout.minimumWidth: root.exportTypeColumnWidth
                                            Layout.maximumWidth: root.exportTypeColumnWidth
                                            model: root.exportOptionsForAnalysisType(annualAnalysisEntry.modelData.analysisType)
                                            currentIndex: Math.max(0, model.indexOf(annualAnalysisEntry.modelData.exportType))
                                            onActivated: {
                                                const selectedExportType = model && currentIndex >= 0 ? model[currentIndex] : ""
                                                root.updateAnnualAnalysisExportType(annualEntryItem.entryIndex, annualAnalysisEntry.index, selectedExportType)
                                            }
                                        }

                                        Item {
                                            Layout.preferredWidth: root.removeColumnWidth
                                            Layout.minimumWidth: root.removeColumnWidth
                                            Layout.maximumWidth: root.removeColumnWidth
                                            Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                                            Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                                            Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                                        }
                                    }
                                }
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

                Item {
                    Layout.preferredWidth: root.leadingColumnWidth
                    Layout.minimumWidth: root.leadingColumnWidth
                    Layout.maximumWidth: root.leadingColumnWidth
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                    Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                    Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                }

                Rectangle {
                    Layout.preferredWidth: root.kindColumnWidth
                    Layout.preferredHeight: root.theme.controlHeight
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.width: root.theme.borderWidthThin
                    border.color: root.theme.borderSoft

                    Label {
                        anchors.fill: parent
                        text: qsTr("Analysis")
                        color: root.theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                }

                Controls.DropdownMenu {
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.analysisNameMinWidth
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

                Controls.DropdownMenu {
                    Layout.preferredWidth: root.exportTypeColumnWidth
                    Layout.minimumWidth: root.exportTypeColumnWidth
                    Layout.maximumWidth: root.exportTypeColumnWidth
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

                Controls.SecondaryButton {
                    text: "×"
                    Layout.preferredWidth: root.removeColumnWidth
                    Layout.minimumWidth: root.removeColumnWidth
                    Layout.maximumWidth: root.removeColumnWidth
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize
                    Layout.minimumHeight: root.theme.viewCompactActionButtonSize
                    Layout.maximumHeight: root.theme.viewCompactActionButtonSize
                    textColor: root.theme.textMuted
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
        root.ensurePendingSelection()
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
        root.ensurePendingSelection()
    }

    Component.onCompleted: root.ensurePendingSelection()
}
