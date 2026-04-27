import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Constants 1.0 as Constants
import FossRedder.Constants 1.0 as FileFormats
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var analysisController: root.appContext ? root.appContext.analysisController : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false

    readonly property var plotTypeOptions: [
        { value: Constants.Analysis.chartTypes.pie, label: qsTr("Pie chart") },
        { value: Constants.Analysis.chartTypes.histogram, label: qsTr("Histogram") }
    ]
    readonly property bool isEdit: root.session
                                 && root.session.selectedAnalysisId
                                 && String(root.session.selectedAnalysisId).length > 0
    readonly property var exportFormatOptions: root.currentMainType() === "plot"
                                               ? [
                                                   { value: "png", label: qsTr("PNG") },
                                                   { value: "jpg", label: qsTr("JPG") }
                                               ]
                                               : [
                                                   { value: FileFormats.exportFormats.xlsx.extension, label: qsTr("XLSX") },
                                                   { value: FileFormats.exportFormats.csv.extension, label: qsTr("CSV") }
                                               ]

    property bool filterEditMode: !root.isEdit
    property var selectedPropertyIds: []
    property var selectedContractTypes: []
    property string allocatableMode: "all"

    property var previewTransactions: []
    property var previewMetrics: ({ statementCount: 0, transactionCount: 0, amountSum: 0.0 })

    property var selectedAdjustmentTxIds: []
    property var adjustmentAmountsById: ({})
    property string pendingAdjustmentsJson: "{}"
    property string calcName: ""
    property string calcPercentText: ""
    property int filterWorkspaceIndex: 0
    property string exportFormat: ""
    property bool includeCalcAdjustments: true
    property string exportStateJson: "{}"
    property string snapshotTransactionsJson: "{}"

    function mapTypeToUi(typeValue) {
        return typeValue === "tab" ? "table" : "plot"
    }

    function mapUiToType(uiValue) {
        return uiValue === "table" ? "tab" : "plot"
    }

    function toggleFilterWorkspace() {
        root.filterWorkspaceIndex = root.filterWorkspaceIndex === 0 ? 1 : 0
    }

    function parseJson(value, fallbackValue) {
        if (!value || String(value).length === 0)
            return fallbackValue
        try { return JSON.parse(value) } catch (e) { return fallbackValue }
    }

    function normalizeExportStateJson(value) {
        if (!value || String(value).length === 0)
            return "{}"
        try {
            const parsed = JSON.parse(value)
            return JSON.stringify(parsed && typeof parsed === "object" ? parsed : {})
        } catch (e) {
            return "{}"
        }
    }

    function parseFilterSpecForUi(spec) {
        const state = {
            dateMode: "range",
            year: "",
            dateFrom: "",
            dateTo: "",
            propertyIds: [],
            contractTypes: [],
            allocatableMode: "all"
        }
        if (!spec || String(spec).length === 0)
            return state

        const clauses = String(spec).split(";")
        for (let i = 0; i < clauses.length; ++i) {
            const clause = clauses[i]
            if (clause.indexOf("date>=") === 0)
                state.dateFrom = clause.substring(6)
            else if (clause.indexOf("date<=") === 0)
                state.dateTo = clause.substring(6)
            else if (clause.indexOf("propertyId=") === 0)
                state.propertyIds = clause.substring(11).split(",").filter(v => v.length > 0)
            else if (clause.indexOf("contract.type=") === 0)
                state.contractTypes = clause.substring(14).split(",").filter(v => v.length > 0)
            else if (clause.indexOf("allocatable=") === 0)
                state.allocatableMode = clause.substring(12)
        }

        if (state.dateFrom.length === 10 && state.dateTo.length === 10
                && state.dateFrom.endsWith("-01-01")
                && state.dateTo.endsWith("-12-31")
                && state.dateFrom.substring(0, 4) === state.dateTo.substring(0, 4)) {
            state.dateMode = "year"
            state.year = state.dateFrom.substring(0, 4)
        }

        return state
    }

    function currentDateMode() {
        return dateFilter.dateModeIndex === 0 ? "year" : "range"
    }

    function currentMainType() {
        return mainTypeCombo.currentIndex === 1 ? "table" : "plot"
    }

    function currentFilterSpec() {
        if (!root.analysisController)
            return ""
        return root.analysisController.analysisFilterSpec(
            root.currentDateMode(),
            dateFilter.yearValue,
            dateFilter.dateFromValue,
            dateFilter.dateToValue,
            root.selectedPropertyIds,
            root.selectedContractTypes,
            root.allocatableMode)
    }

    function ensureChoices() {
        const propertyRows = root.session ? root.session.propertyRows() : []
        propertyFilterRows = propertyRows

        let types = []
        try { types = root.analysisController ? root.analysisController.contractTypes() : [] } catch (e) { types = [] }
        const dedup = []
        const seen = ({})
        for (let i = 0; i < types.length; ++i) {
            const value = String(types[i])
            if (value.length === 0 || seen[value])
                continue
            seen[value] = true
            dedup.push(value)
        }
        contractTypeRows = dedup

        if (root.selectedPropertyIds.length === 0)
            root.selectedPropertyIds = propertyRows.map(p => p.id)
        if (root.selectedContractTypes.length === 0)
            root.selectedContractTypes = dedup.slice()
    }

    function refreshPreview() {
        if (!root.analysisController)
            return
        const preview = root.analysisController.previewTransactions(root.currentFilterSpec())
        root.previewTransactions = preview && preview.transactions ? preview.transactions : []
        root.previewMetrics = preview && preview.metrics ? preview.metrics : ({ statementCount: 0, transactionCount: 0, amountSum: 0.0 })
    }

    function resetAdjustments() {
        root.selectedAdjustmentTxIds = []
        root.adjustmentAmountsById = ({})
        root.pendingAdjustmentsJson = "{}"
        root.calcName = ""
        root.calcPercentText = ""
    }

    function applySelectedCalc() {
        let taxPercent = parseFloat(root.calcPercentText)
        if (isNaN(taxPercent))
            taxPercent = 0.0
        root.pendingAdjustmentsJson = root.analysisController
                ? root.analysisController.analysisAdjustmentsJson(root.previewTransactions, root.selectedAdjustmentTxIds, taxPercent)
                : "{}"
        root.adjustmentAmountsById = root.parseJson(root.pendingAdjustmentsJson, {})
    }

    function parseAnalysisIntoForm() {
        const selectedId = root.session ? (root.session.selectedAnalysisId || "") : ""
        const row = root.analysisRowById(selectedId)

        if (!row || !row.id) {
            nameField.text = ""
            mainTypeCombo.currentIndex = 0
            plotSubtypeCombo.currentIndex = 0
            dateFilter.dateModeIndex = 1
            dateFilter.yearValue = ""
            dateFilter.dateFromValue = ""
            dateFilter.dateToValue = ""
            root.selectedPropertyIds = propertyFilterRows.map(p => p.id)
            root.selectedContractTypes = contractTypeRows.slice()
            root.allocatableMode = "all"
            root.filterEditMode = true
            root.filterWorkspaceIndex = 0
            root.exportFormat = ""
            root.includeCalcAdjustments = true
            root.exportStateJson = "{}"
            root.snapshotTransactionsJson = "{}"
            root.resetAdjustments()
            root.refreshPreview()
            return
        }

        nameField.text = row.name ? row.name : ""
        const uiType = root.mapTypeToUi(row.type ? row.type : "plot")
        mainTypeCombo.currentIndex = uiType === "table" ? 1 : 0

        const config = root.parseJson(row.config ? row.config : "{}", {})
        plotSubtypeCombo.currentIndex = config.plotType === "histogram" ? 1 : 0

        const parsedFilter = root.parseFilterSpecForUi(row.filter ? row.filter : "")
        dateFilter.dateModeIndex = parsedFilter.dateMode === "year" ? 0 : 1
        dateFilter.yearValue = parsedFilter.year
        dateFilter.dateFromValue = parsedFilter.dateFrom
        dateFilter.dateToValue = parsedFilter.dateTo
        root.selectedPropertyIds = parsedFilter.propertyIds.length > 0 ? parsedFilter.propertyIds : propertyFilterRows.map(p => p.id)
        root.selectedContractTypes = parsedFilter.contractTypes.length > 0 ? parsedFilter.contractTypes : contractTypeRows.slice()
        root.allocatableMode = parsedFilter.allocatableMode

        root.pendingAdjustmentsJson = row && row.adjustments ? row.adjustments : "{}"
        root.adjustmentAmountsById = root.parseJson(root.pendingAdjustmentsJson, {})
        root.exportFormat = row && row.exportFormat ? row.exportFormat : ""
        root.includeCalcAdjustments = row && row.includeCalcAdjustments !== undefined ? !!row.includeCalcAdjustments : true
        root.exportStateJson = root.normalizeExportStateJson(row && row.exportState ? row.exportState : "{}")
        root.snapshotTransactionsJson = row && row.snapshotTransactions ? row.snapshotTransactions : "{}"
        if (!root.exportFormat || root.exportFormat.length === 0)
            root.exportFormat = root.resolvedExportFormat()

        root.filterEditMode = false
        if (root.analysisController) {
            const result = root.analysisController.computeAnalysis(row.id, root.currentFilterSpec())
            if (result && Object.keys(result).length > 0)
                root.session.lastAnalysisResult = result
        }
        root.refreshPreview()
    }

    function submitAnalysis(isCreate) {
        if (!root.analysisController || !root.session)
            return

        const strategyType = root.mapUiToType(root.currentMainType())
        const selectedPlotType = root.plotTypeOptions[Math.max(0, plotSubtypeCombo.currentIndex)].value
        const configJson = root.analysisController.analysisConfigJson(strategyType,
                                                                      selectedPlotType,
                                                                      "totalAmount",
                                                                      root.selectedPropertyIds,
                                                                      root.selectedContractTypes,
                                                                      0.0)
        const filterSpec = root.currentFilterSpec()
        const selectedExportFormat = root.resolvedExportFormat()

        if (isCreate) {
            root.snapshotTransactionsJson = JSON.stringify(root.previewTransactions)
            const newId = root.analysisController.createAnalysis(nameField.text,
                                                                 strategyType,
                                                                 configJson,
                                                                 filterSpec,
                                                                 selectedExportFormat,
                                                                 root.includeCalcAdjustments,
                                                                 root.normalizeExportStateJson(root.exportStateJson),
                                                                 root.snapshotTransactionsJson)
            if (!newId || newId.length === 0)
                return
            root.session.selectedAnalysisId = newId
        } else {
            const selectedId = root.session.selectedAnalysisId ? String(root.session.selectedAnalysisId) : ""
            if (selectedId.length === 0)
                return
            root.analysisController.updateAnalysis(selectedId,
                                                   nameField.text,
                                                   strategyType,
                                                   configJson,
                                                   filterSpec,
                                                   selectedExportFormat,
                                                   root.includeCalcAdjustments,
                                                   root.normalizeExportStateJson(root.exportStateJson),
                                                   root.snapshotTransactionsJson)
        }

        const currentId = root.session.selectedAnalysisId ? String(root.session.selectedAnalysisId) : ""
        if (currentId.length > 0 && root.pendingAdjustmentsJson && root.session.analyses)
            root.session.analyses.setAdjustmentsById(currentId, root.pendingAdjustmentsJson)

        const result = root.analysisController.computeAnalysis(root.session.selectedAnalysisId, filterSpec)
        if (result && Object.keys(result).length > 0)
            root.session.lastAnalysisResult = result

        root.filterEditMode = false
    }

    function deleteCurrentAnalysis() {
        const selectedId = root.session ? (root.session.selectedAnalysisId || "") : ""
        if (!selectedId || selectedId.length === 0 || !root.analysisController)
            return

        const currentIndex = root.analysisIndexById(selectedId)
        root.analysisController.deleteAnalysis(selectedId)

        const total = root.analysesCount()
        if (total <= 0) {
            root.session.selectedAnalysisId = ""
            return
        }

        const nextIndex = Math.min(currentIndex, total - 1)
        const nextRow = root.analysisRowAt(nextIndex)
        root.session.selectedAnalysisId = nextRow && nextRow.id ? String(nextRow.id) : ""
    }

    function currentSelectedType() {
        const selectedId = root.session.selectedAnalysisId ? String(root.session.selectedAnalysisId) : ""
        const row = root.analysisRowById(selectedId)
        return row && row.type ? row.type : "plot"
    }

    function analysisRowById(id) {
        if (!id || String(id).length === 0)
            return null
        const rows = root.currentAnalysisRows()
        const targetId = String(id)
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (row && row.id === targetId)
                return row
        }
        return null
    }

    function currentAnalysisRows() {
        return root.session ? root.session.analysisRows() : []
    }

    function analysesCount() {
        return root.currentAnalysisRows().length
    }

    function analysisIndexById(id) {
        if (!id || String(id).length === 0)
            return -1
        const rows = root.currentAnalysisRows()
        const targetId = String(id)
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (row && row.id === targetId)
                return i
        }
        return -1
    }

    function analysisRowAt(index) {
        const rows = root.currentAnalysisRows()
        if (index < 0 || index >= rows.length)
            return null
        return rows[index]
    }

    function refreshFromSelection() {
        root.ensureChoices()
        root.parseAnalysisIntoForm()
    }

    function navigateAnalysis(delta) {
        if (!root.session)
            return

        const total = root.analysesCount()
        if (total <= 0)
            return

        const currentId = root.session.selectedAnalysisId ? String(root.session.selectedAnalysisId) : ""
        let index = root.analysisIndexById(currentId)
        if (index < 0)
            index = delta > 0 ? -1 : 0

        const nextIndex = (index + delta + total) % total
        const nextRow = root.analysisRowAt(nextIndex)
        if (nextRow && nextRow.id)
            root.session.selectedAnalysisId = String(nextRow.id)
    }

    function clearFilters() {
        dateFilter.dateModeIndex = 1
        dateFilter.yearValue = ""
        dateFilter.dateFromValue = ""
        dateFilter.dateToValue = ""
        root.selectedPropertyIds = propertyFilterRows.map(p => p.id)
        root.selectedContractTypes = contractTypeRows.slice()
        root.allocatableMode = "all"
        root.resetAdjustments()
        root.refreshPreview()
    }

    function hasCustomExportState() {
        const state = root.parseJson(root.normalizeExportStateJson(root.exportStateJson), {})
        const hasPieFilter = state.pieLegendFilter && state.pieLegendFilter.length > 0
        const hasHistogramSplit = state.histogramSplitByProperty === true
        return !!hasPieFilter || !!hasHistogramSplit
    }

    function resetPreviewExportState() {
        root.exportStateJson = "{}"
    }

    function resolvedExportFormat() {
        if (root.exportFormat && String(root.exportFormat).length > 0)
            return root.exportFormat
        return root.exportFormatOptions.length > 0 ? root.exportFormatOptions[0].value : ""
    }

    function exportFormatIndexForValue(value) {
        for (let i = 0; i < root.exportFormatOptions.length; ++i) {
            if (root.exportFormatOptions[i].value === value)
                return i
        }
        return 0
    }

    function syncExportFormatCombo() {
        if (!exportFormatCombo)
            return
        const nextIndex = root.exportFormatIndexForValue(root.resolvedExportFormat())
        if (exportFormatCombo.currentIndex !== nextIndex)
            exportFormatCombo.currentIndex = nextIndex
    }

    property var propertyFilterRows: []
    property var contractTypeRows: []

    Timer {
        id: previewDebounce
        interval: 40
        repeat: false
        running: false
        onTriggered: root.refreshPreview()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.viewFormSpacing

        Flickable {
            id: analysisScroll
            Layout.fillWidth: true
            Layout.fillHeight: root.isEdit
            Layout.minimumHeight: 0
            Layout.preferredHeight: root.isEdit ? -1 : analysisContent.implicitHeight
            clip: true
            contentWidth: width
            contentHeight: root.isEdit ? height : analysisContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            ColumnLayout {
                id: analysisContent
                width: analysisScroll.width
                height: root.isEdit ? analysisScroll.height : implicitHeight
                spacing: root.theme.viewFormSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Analysis Name"); Layout.preferredWidth: root.theme.formLabelWidth }
                    Controls.TextField { id: nameField; Layout.fillWidth: true; placeholderText: "" }
                }

                Controls.Panel {
                    visible: !root.isEdit
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Analysis Type"); Layout.preferredWidth: root.theme.formLabelWidth }
                            Controls.ComboBox { id: mainTypeCombo; Layout.fillWidth: true; model: [ qsTr("Plot"), qsTr("Table") ]; currentIndex: 0 }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            visible: root.currentMainType() === "plot"
                            Label { text: qsTr("Plot Subtype"); Layout.preferredWidth: root.theme.formLabelWidth }
                            Controls.ComboBox { id: plotSubtypeCombo; Layout.fillWidth: true; model: root.plotTypeOptions; textRole: "label"; currentIndex: 0 }
                        }
                    }
                }

                Controls.Panel {
                    visible: root.isEdit
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Export Format"); Layout.preferredWidth: root.theme.formLabelWidth }
                            Controls.ComboBox {
                                id: exportFormatCombo
                                Layout.preferredWidth: root.theme.formFieldWidth
                                model: root.exportFormatOptions
                                textRole: "label"
                                currentIndex: 0
                                onCurrentIndexChanged: {
                                    if (currentIndex >= 0 && currentIndex < root.exportFormatOptions.length)
                                        root.exportFormat = root.exportFormatOptions[currentIndex].value
                                }
                            }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Include Calc Adjustments"); Layout.preferredWidth: root.theme.formLabelWidth }
                            Controls.CheckBox {
                                checked: root.includeCalcAdjustments
                                onClicked: root.includeCalcAdjustments = checked
                            }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Preview Export State"); Layout.preferredWidth: root.theme.formLabelWidth }
                            Label {
                                text: root.hasCustomExportState() ? qsTr("Customized") : qsTr("Default")
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Controls.Panel {
                    Layout.fillWidth: true
                    Layout.fillHeight: root.isEdit
                    Layout.minimumHeight: 0
                    Layout.preferredHeight: -1
                    visible: root.isEdit

                    Loader {
                        id: resultLoader
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceComponent: {
                            const t = root.currentSelectedType()
                            return t === "tab" ? tableComp : plotComp
                        }
                    }
                }
            }
        }

        Controls.Panel {
            visible: !root.isEdit && root.filterEditMode
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.theme.spacingSmall

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.filterWorkspaceIndex

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Flickable {
                            id: filterSpecScroll
                            anchors.fill: parent
                            clip: true
                            contentWidth: width
                            contentHeight: filterSpecContent.implicitHeight
                            boundsBehavior: Flickable.StopAtBounds

                            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                            ColumnLayout {
                                id: filterSpecContent
                                width: filterSpecScroll.width
                                spacing: root.theme.spacingSmall

                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }

                                Views.AnalysisDateFilter {
                                    id: dateFilter
                                    theme: root.theme
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: implicitHeight
                                    onFilterChanged: previewDebounce.restart()
                                }

                                Views.AnalysisPropertyFilter {
                                    theme: root.theme
                                    propertyRows: root.propertyFilterRows
                                    selectedIds: root.selectedPropertyIds
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: implicitHeight
                                    onSelectionChanged: (ids) => { root.selectedPropertyIds = ids; previewDebounce.restart() }
                                }

                                Views.AnalysisContractTypeFilter {
                                    theme: root.theme
                                    contractTypes: root.contractTypeRows
                                    selectedTypes: root.selectedContractTypes
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: implicitHeight
                                    onSelectionChanged: (selected) => { root.selectedContractTypes = selected; previewDebounce.restart() }
                                }

                                Views.AnalysisAllocatableFilter {
                                    theme: root.theme
                                    mode: root.allocatableMode
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: implicitHeight
                                    onModeSelected: (mode) => { root.allocatableMode = mode; previewDebounce.restart() }
                                }

                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: root.theme.spacingSmall
                                }
                            }
                        }
                    }

                    Views.AnalysisTransactionsPanel {
                        theme: root.theme
                        transactions: root.previewTransactions
                        metrics: root.previewMetrics
                        selectedTransactionIds: root.selectedAdjustmentTxIds
                        adjustedAmountsById: root.adjustmentAmountsById
                        calcName: root.calcName
                        calcPercentText: root.calcPercentText
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onSelectionChanged: (ids) => root.selectedAdjustmentTxIds = ids
                        onCalcNameEdited: (name) => root.calcName = name
                        onCalcPercentEdited: (percent) => root.calcPercentText = percent
                        onApplyCalcRequested: root.applySelectedCalc()
                    }
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                text: "◀"
                enabled: root.analysesCount() > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateAnalysis(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                visible: !root.isEdit && root.filterEditMode
                text: "⇆"
                bordered: true
                Layout.preferredWidth: 48
                onClicked: root.toggleFilterWorkspace()
            }

            Controls.DangerButton {
                visible: !root.isEdit
                text: qsTr("Reset")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFilters()
            }

            Controls.SuccessButton {
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitAnalysis(true)
            }

            Controls.DangerButton {
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteCurrentAnalysis()
            }

            Controls.SuccessButton {
                visible: root.isEdit
                text: qsTr("Update")
                enabled: nameField.text.length > 0
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitAnalysis(false)
            }

            Item { Layout.fillWidth: true }

            Controls.Button {
                text: "▶"
                enabled: root.analysesCount() > 0
                Layout.preferredWidth: root.theme.viewNavigationButtonWidth
                bordered: true
                onClicked: root.navigateAnalysis(1)
            }
        }
    }

    Component {
        id: plotComp
        Views.AnalysisPlotView {
            appContext: root.appContext
            theme: root.theme
            exportStateJson: root.exportStateJson
            onExportStateChanged: (stateJson) => root.exportStateJson = root.normalizeExportStateJson(stateJson)
        }
    }
    Component { id: tableComp; Views.AnalysisTableView { appContext: root.appContext; theme: root.theme } }

    Connections {
        target: root.session
        function onSelectedAnalysisIdChanged() { root.parseAnalysisIntoForm() }
    }

    Component.onCompleted: {
        root.ensureChoices()
        root.parseAnalysisIntoForm()
    }
}
