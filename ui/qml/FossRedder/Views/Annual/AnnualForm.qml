/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualForm.qml
 * @brief Provides the AnnualForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var sessionState: root.appContext ? root.appContext.sessionState : null
    readonly property int workspaceRevision: (root.session && root.session.dataRevision !== undefined)
                                             ? root.session.dataRevision
                                             : 0
    readonly property string currentAnnualId: root.session && root.session.selectedAnnualId
                                            ? String(root.session.selectedAnnualId)
                                            : ""
    property bool isEdit: root.currentAnnualId.length > 0
    property string annualNameText: ""
    property string yearText: String(new Date().getFullYear() - 1)
    property var analysisIds: []
    property var assignedAnalysisRows: []
    property int annualWorkspaceIndex: 0
    property var annualVerificationIssues: ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
    property var annualStatusMetrics: ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
    property var annualTransactions: []
    property var annualTransactionGroups: ({ deduplicated: [], similar: [], divergent: [], workspaceOnly: [] })
    property bool annualRebuildInProgress: false
    property bool annualRebuildPending: false
    property string savedAnnualNameText: ""
    property string savedYearText: ""
    property var savedAnalysisIds: []
    property bool analysisMetadataDirty: false
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    anchors.fill: parent

    function annualRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.annualRows() : []
    }

    function analysisRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.analysisRows() : []
    }

    function parseYear(text) {
        const year = parseInt(String(text || "").trim())
        return isNaN(year) ? -1 : year
    }

    function defaultAnnualYearText() {
        return String(new Date().getFullYear() - 1)
    }

    function canSubmit() {
        return root.parseYear(root.yearText) > 0
    }

    function normalizedList(values) {
        const list = root.toJsArray(values)
        list.sort()
        return JSON.stringify(list)
    }

    function toJsArray(values) {
        const out = []
        if (!values || values.length === undefined)
            return out
        for (let i = 0; i < values.length; ++i)
            out.push(values[i])
        return out
    }

    function captureSavedState() {
        root.savedAnnualNameText = String(root.annualNameText || "")
        root.savedYearText = String(root.yearText || "")
        root.savedAnalysisIds = root.toJsArray(root.analysisIds)
        root.analysisMetadataDirty = false
    }

    function hasChanges() {
        if (!root.isEdit)
            return root.canSubmit()
        return root.savedAnnualNameText !== String(root.annualNameText || "")
                || root.savedYearText !== String(root.yearText || "")
                || root.normalizedList(root.savedAnalysisIds) !== root.normalizedList(root.analysisIds)
                || root.analysisMetadataDirty
    }

    function clearFields() {
        root.annualNameText = ""
        root.yearText = root.defaultAnnualYearText()
        root.analysisIds = []
        root.assignedAnalysisRows = []
        root.annualWorkspaceIndex = 0
        root.annualVerificationIssues = ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
        root.annualStatusMetrics = ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
        root.annualTransactions = []
        root.annualTransactionGroups = ({ deduplicated: [], similar: [], divergent: [], workspaceOnly: [] })
        root.analysisMetadataDirty = false
    }

    function applyYearSelection(nextValue) {
        const nextYear = String(nextValue)
        if (root.yearText === nextYear)
            return
        root.yearText = nextYear
        root.rebuildAnnualResultState()
    }

    function requestAnnualRebuild() {
        if (root.annualRebuildInProgress) {
            root.annualRebuildPending = true
            return
        }
        root.rebuildAnnualResultState()
    }

    function rebuildAnnualResultState() {
        if (root.annualRebuildInProgress) {
            root.annualRebuildPending = true
            return
        }
        root.annualRebuildInProgress = true
        root.annualRebuildPending = false
        if (!root.workspaceFacade) {
            root.annualVerificationIssues = ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
            root.annualStatusMetrics = ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
            root.annualTransactions = []
            root.annualTransactionGroups = ({ deduplicated: [], similar: [], divergent: [], workspaceOnly: [] })
            root.annualRebuildInProgress = false
            return
        }
        const result = root.workspaceFacade.annualResultStatePreview(root.currentAnnualId,
                                                                     root.analysisIds || [],
                                                                     root.parseYear(root.yearText))
        const stats = result && result.stats ? result.stats : ({})
        root.annualVerificationIssues = ({
            missingFromYear: stats && stats.missingFromYear !== undefined ? stats.missingFromYear : 0,
            mixedInAnnual: stats && stats.mixedYear !== undefined ? stats.mixedYear : 0,
            duplicateCount: stats && stats.duplicateCount !== undefined ? stats.duplicateCount : 0,
            missingLive: stats && stats.missingLive !== undefined ? stats.missingLive : 0
        })
        root.annualStatusMetrics = ({
            neutral: stats && stats.neutral !== undefined ? stats.neutral : 0,
            unverified: stats && stats.unverified !== undefined ? stats.unverified : 0,
            verified: stats && stats.verified !== undefined ? stats.verified : 0,
            completed: stats && stats.completed !== undefined ? stats.completed : 0
        })
        root.annualTransactions = result && result.transactions ? result.transactions : []
        let groups = ({
            deduplicated: result && result.deduplicated ? result.deduplicated : [],
            similar: result && result.similar ? result.similar : [],
            divergent: result && result.divergent ? result.divergent : [],
            workspaceOnly: result && result.workspaceOnly ? result.workspaceOnly : []
        })
        if (root.groupedCount(groups) === 0 && root.annualTransactions.length > 0)
            groups = root.groupsFromTransactions(root.annualTransactions)
        root.annualTransactionGroups = groups
        root.annualRebuildInProgress = false
        if (root.annualRebuildPending) {
            root.annualRebuildPending = false
            root.rebuildAnnualResultState()
        }
    }

    function resolveAssignedAnalyses(allAnalysisRows, ids) {
        if (!root.workspaceFacade || !root.workspaceFacade.assignedAnnualAnalysisRows)
            return []
        return root.workspaceFacade.assignedAnnualAnalysisRows(allAnalysisRows || [], ids || [])
    }

    function groupedCount(groups) {
        const g = groups || ({})
        const deduplicated = g.deduplicated && g.deduplicated.length !== undefined ? g.deduplicated.length : 0
        const similar = g.similar && g.similar.length !== undefined ? g.similar.length : 0
        const divergent = g.divergent && g.divergent.length !== undefined ? g.divergent.length : 0
        const workspaceOnly = g.workspaceOnly && g.workspaceOnly.length !== undefined ? g.workspaceOnly.length : 0
        return deduplicated + similar + divergent + workspaceOnly
    }

    function groupsFromTransactions(rows) {
        const allRows = rows && rows.length !== undefined ? rows : []
        const deduplicated = []
        const similar = []
        const divergent = []
        const workspaceOnly = []

        for (let i = 0; i < allRows.length; ++i) {
            const row = allRows[i] || ({})
            const key = String(row.key || "")
            if (key.indexOf("live|") === 0) {
                workspaceOnly.push(row)
                continue
            }
            if (key.indexOf("sim|") === 0 || row.isCalcVariant) {
                similar.push(row)
                continue
            }
            if (key.indexOf("div|") === 0) {
                divergent.push(row)
                continue
            }
            deduplicated.push(row)
        }

        return ({
            deduplicated: deduplicated,
            similar: similar,
            divergent: divergent,
            workspaceOnly: workspaceOnly
        })
    }

    function availableAnalysisRows() {
        if (!root.workspaceFacade || !root.workspaceFacade.availableAnnualAnalysisRows)
            return []
        return root.workspaceFacade.availableAnnualAnalysisRows(root.analysisRows() || [], root.analysisIds || [])
    }

    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }

        if (!root.workspaceFacade || !root.isEdit) {
            root.clearFields()
            return
        }

        const payload = root.workspaceFacade.annual(root.currentAnnualId)
        root.annualNameText = payload && payload.name !== undefined ? String(payload.name) : ""
        root.yearText = payload && payload.year !== undefined ? String(payload.year) : ""
        root.analysisIds = (root.workspaceFacade && root.workspaceFacade.normalizeAnalysisIds)
                ? root.workspaceFacade.normalizeAnalysisIds(payload && payload.analysisIds ? payload.analysisIds : [])
                : (payload && payload.analysisIds ? payload.analysisIds : [])
        root.assignedAnalysisRows = root.resolveAssignedAnalyses(root.analysisRows(), root.analysisIds)
        if (root.appContext && root.appContext.isDebugBuild)
            console.log("[AnnualForm] syncFields", "analysisIds=", JSON.stringify(root.analysisIds), "assignedRows=", root.assignedAnalysisRows.length)
        root.requestAnnualRebuild()
        root.captureSavedState()
    }

    function navigateAnnual(delta) {
        const rows = root.annualRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isEdit ? (root.session.selectedAnnualId || "") : ""
        const currentIndex = root.sessionState.indexOfId ? root.sessionState.indexOfId(rows, currentId) : -1
        if ((delta > 0 && currentIndex === rows.length - 1)
                || (delta < 0 && currentIndex === 0)) {
            root.session.selectedAnnualId = ""
            return
        }
        const nextIndex = currentIndex < 0
            ? (delta > 0 ? 0 : rows.length - 1)
            : currentIndex + delta
        const nextId = rows[nextIndex] && rows[nextIndex].id ? String(rows[nextIndex].id) : ""
        if (!nextId || nextId.length === 0)
            return

        root.session.selectedAnnualId = nextId
    }

    function submitAnnual() {
        if (!root.workspaceFacade)
            return

        const year = root.parseYear(root.yearText)
        if (year <= 0)
            return

        const annualId = root.workspaceFacade.saveAnnual(root.currentAnnualId,
                                                          root.annualNameText,
                                                          year,
                                                          root.analysisIds || [])

        if (!annualId || annualId.length === 0)
            return

        if (root.session)
            root.session.selectedAnnualId = annualId
        root.captureSavedState()
    }

    function deleteAnnual() {
        if (!root.workspaceFacade || !root.isEdit)
            return

        const removedId = root.currentAnnualId
        root.workspaceFacade.deleteAnnual(removedId)
        if (!root.session) {
            root.clearFields()
            return
        }

        const nextId = root.sessionState.deleteNextSelectionId(root.annualRows(), removedId, 0, "id")
        root.session.selectedAnnualId = nextId || ""
        if (!nextId || nextId.length === 0)
            root.clearFields()
    }

    onCurrentAnnualIdChanged: root.syncFields()
    onIsEditChanged: root.syncFields()
    Connections {
        target: root.session
        function onSelectedAnnualIdChanged() { root.syncFields() }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.viewFormSpacing

        Flickable {
            id: annualFormScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: annualWorkspaceContent.implicitHeight
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: annualWorkspaceContent
                width: annualFormScroll.width
                height: Math.max(implicitHeight, annualFormScroll.height)
                spacing: root.theme.viewFormSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Annual Name")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    Controls.TextField {
                        objectName: "annualNameField"
                        Layout.fillWidth: true
                        text: root.annualNameText
                        onTextEdited: root.annualNameText = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Year")
                        Layout.preferredWidth: root.theme.formLabelWidth
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Controls.SecondaryButton {
                            objectName: "annualYearDecreaseButton"
                            Layout.preferredWidth: root.theme.controlHeight
                            Layout.preferredHeight: root.theme.controlHeight
                            Layout.minimumWidth: root.theme.controlHeight
                            Layout.maximumWidth: root.theme.controlHeight
                            text: "\u25BC"
                            onClicked: {
                                const currentYear = root.parseYear(root.yearText) > 0
                                                    ? root.parseYear(root.yearText)
                                                    : parseInt(root.defaultAnnualYearText())
                                root.applyYearSelection(Math.max(1900, currentYear - 1))
                            }
                        }

                        Rectangle {
                            objectName: "annualYearField"
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.theme.controlHeight
                            radius: root.theme.radius
                            color: root.theme.surface
                            border.color: root.theme.borderMedium
                            border.width: root.theme.borderWidthThin

                            Label {
                                anchors.fill: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: root.yearText
                            }
                        }

                        Controls.SecondaryButton {
                            objectName: "annualYearIncreaseButton"
                            Layout.preferredWidth: root.theme.controlHeight
                            Layout.preferredHeight: root.theme.controlHeight
                            Layout.minimumWidth: root.theme.controlHeight
                            Layout.maximumWidth: root.theme.controlHeight
                            text: "\u25B2"
                            onClicked: {
                                const currentYear = root.parseYear(root.yearText) > 0
                                                    ? root.parseYear(root.yearText)
                                                    : parseInt(root.defaultAnnualYearText())
                                root.applyYearSelection(Math.min(2500, currentYear + 1))
                            }
                        }
                    }
                }

                StackLayout {
                    id: annualWorkspaceStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.annualWorkspaceIndex

                    Views.AnnualAnalysesPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.bottomMargin: root.theme.spacingSmall
                        appContext: root.appContext
                        theme: root.theme
                        allAnalysisRows: root.availableAnalysisRows()
                        analysisRows: root.assignedAnalysisRows
                        selectedAnalysisIds: root.analysisIds
                        analysisMapper: root.workspaceFacade
                        onSelectionChanged: function(ids) {
                            root.analysisIds = (root.workspaceFacade && root.workspaceFacade.normalizeAnalysisIds)
                                    ? root.workspaceFacade.normalizeAnalysisIds(ids || [])
                                    : (ids || [])
                            root.assignedAnalysisRows = root.resolveAssignedAnalyses(root.analysisRows(), root.analysisIds)
                            if (root.appContext && root.appContext.isDebugBuild)
                                console.log("[AnnualForm] onSelectionChanged", "ids=", JSON.stringify(ids), "normalized=", JSON.stringify(root.analysisIds), "assignedRows=", root.assignedAnalysisRows.length)
                            root.requestAnnualRebuild()
                        }
                        onAnalysisOptionsChanged: {
                            root.analysisMetadataDirty = true
                            root.requestAnnualRebuild()
                        }
                    }

                    Views.AnnualTransactionsPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        theme: root.theme
                        transactions: root.annualTransactions
                        groupedTransactions: root.annualTransactionGroups
                    }
                }

                Views.AnnualVerificationPanel {
                    Layout.fillWidth: true
                    theme: root.theme
                    assignedAnalysisCount: root.analysisIds ? root.analysisIds.length : 0
                    transactionCount: root.annualTransactions ? root.annualTransactions.length : 0
                    issues: root.annualVerificationIssues
                    statusMetrics: root.annualStatusMetrics
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.PrevButton {
                objectName: "annualPreviousButton"
                enabled: root.annualRows().length > 0
                onClicked: root.navigateAnnual(-1)
            }

            Item { Layout.fillWidth: true }

            Controls.SecondaryButton {
                objectName: "annualToggleWorkspaceButton"
                text: "⇆"
                Layout.preferredWidth: 48
                onClicked: root.annualWorkspaceIndex = root.annualWorkspaceIndex === 0 ? 1 : 0
            }

            Controls.DangerButton {
                objectName: "annualClearButton"
                visible: !root.isEdit
                text: qsTr("Clear")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.clearFields()
            }

            Controls.SuccessButton {
                objectName: "annualCreateButton"
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: root.canSubmit()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitAnnual()
            }

            Controls.DangerButton {
                objectName: "annualDeleteButton"
                visible: root.isEdit
                text: qsTr("Delete")
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.deleteAnnual()
            }

            Controls.SuccessButton {
                objectName: "annualUpdateButton"
                visible: root.isEdit
                text: qsTr("Update")
                enabled: root.canSubmit() && root.hasChanges()
                Layout.preferredWidth: root.theme.viewActionButtonWidth
                onClicked: root.submitAnnual()
            }

            Item { Layout.fillWidth: true }

            Controls.NextButton {
                objectName: "annualNextButton"
                enabled: root.annualRows().length > 0
                onClicked: root.navigateAnnual(1)
            }
        }
    }

    Component.onCompleted: root.syncFields()
}
