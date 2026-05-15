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
    readonly property var current: root.session ? root.session.selectedAnnual : null
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0
    property string annualNameText: ""
    property string yearText: ""
    property var analysisIds: []
    property var assignedAnalysisRows: []
    property int annualWorkspaceIndex: 0
    property var annualVerificationIssues: ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
    property var annualStatusMetrics: ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
    property var annualTransactions: []
    property string savedAnnualNameText: ""
    property string savedYearText: ""
    property var savedAnalysisIds: []
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false
    anchors.fill: parent

    function annualRows() {
        return root.session ? root.session.annualRows() : []
    }

    function analysisRows() {
        return root.session ? root.session.analysisRows() : []
    }

    function parseYear(text) {
        const year = parseInt(String(text || "").trim())
        return isNaN(year) ? -1 : year
    }

    function canSubmit() {
        return root.parseYear(root.yearText) > 0
    }

    function normalizedList(values) {
        const list = values ? values.slice() : []
        list.sort()
        return JSON.stringify(list)
    }

    function captureSavedState() {
        root.savedAnnualNameText = String(root.annualNameText || "")
        root.savedYearText = String(root.yearText || "")
        root.savedAnalysisIds = root.analysisIds ? root.analysisIds.slice() : []
    }

    function hasChanges() {
        if (!root.isEdit)
            return root.canSubmit()
        return root.savedAnnualNameText !== String(root.annualNameText || "")
                || root.savedYearText !== String(root.yearText || "")
                || root.normalizedList(root.savedAnalysisIds) !== root.normalizedList(root.analysisIds)
    }

    function clearFields() {
        root.annualNameText = ""
        root.yearText = ""
        root.analysisIds = []
        root.assignedAnalysisRows = []
        root.annualWorkspaceIndex = 0
        root.annualVerificationIssues = ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
        root.annualStatusMetrics = ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })
        root.annualTransactions = []
    }

    function transactionStatusText(value) {
        const status = Number(value)
        if (status === 1)
            return qsTr("Unverified")
        if (status === 2)
            return qsTr("Verified")
        if (status === 3)
            return qsTr("Completed")
        return qsTr("Neutral")
    }

    function bookingYear(bookingDate) {
        const text = String(bookingDate || "")
        if (text.length >= 4)
            return parseInt(text.substring(0, 4))
        return -1
    }

    function parseSnapshotTransactions(snapshotTransactionsJson) {
        if (!snapshotTransactionsJson || String(snapshotTransactionsJson).length === 0)
            return []
        try {
            const parsed = JSON.parse(String(snapshotTransactionsJson))
            return Array.isArray(parsed) ? parsed : []
        } catch (e) {
            return []
        }
    }

    function rebuildAnnualDerivedState() {
        const ids = root.analysisIds || []
        const analyses = root.analysisRows() || []
        const snapshotById = ({})
        const duplicateCounter = ({})

        for (let i = 0; i < ids.length; ++i) {
            const analysisId = String(ids[i] || "")
            if (analysisId.length === 0)
                continue

            let analysisRow = null
            for (let j = 0; j < analyses.length; ++j) {
                const row = analyses[j] || ({})
                if (String(row.id || "") === analysisId) {
                    analysisRow = row
                    break
                }
            }
            if (!analysisRow)
                continue

            const snapshotRows = root.parseSnapshotTransactions(analysisRow.snapshotTransactions)
            for (let k = 0; k < snapshotRows.length; ++k) {
                const snapshot = snapshotRows[k] || ({})
                const txId = String(snapshot.id || "")
                if (txId.length === 0)
                    continue
                duplicateCounter[txId] = (duplicateCounter[txId] || 0) + 1
                if (!snapshotById[txId])
                    snapshotById[txId] = snapshot
            }
        }

        const txIds = Object.keys(snapshotById)
        const targetYear = root.parseYear(root.yearText)
        const issues = { missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 }
        const metrics = { neutral: 0, unverified: 0, verified: 0, completed: 0 }
        const rows = []

        for (let i = 0; i < txIds.length; ++i) {
            const txId = txIds[i]
            const snapshot = snapshotById[txId] || ({})
            const live = root.workspaceFacade ? root.workspaceFacade.transaction(txId) : ({})
            const hasLive = live && live.id && String(live.id).length > 0
            const bookingDate = hasLive ? String(live.bookingDate || "") : String(snapshot.date || "")
            const year = root.bookingYear(bookingDate)
            const status = hasLive ? Number(live.status !== undefined ? live.status : 0) : 0

            if (targetYear > 0 && year > 0 && year !== targetYear)
                issues.mixedInAnnual += 1
            if ((duplicateCounter[txId] || 0) > 1)
                issues.duplicateCount += 1
            if (!hasLive)
                issues.missingLive += 1

            if (status === 1)
                metrics.unverified += 1
            else if (status === 2)
                metrics.verified += 1
            else if (status === 3)
                metrics.completed += 1
            else
                metrics.neutral += 1

            rows.push({
                id: txId,
                name: hasLive ? String(live.name || "") : String(snapshot.transactionName || snapshot.name || ""),
                bookingDate: bookingDate,
                amount: hasLive ? Number(live.amount || 0.0) : Number(snapshot.amount || 0.0),
                status: status,
                statusText: root.transactionStatusText(status),
                duplicateCount: duplicateCounter[txId] || 1,
                isDuplicate: (duplicateCounter[txId] || 0) > 1,
                isMixedYear: targetYear > 0 && year > 0 && year !== targetYear,
                isMissingLive: !hasLive,
                statementId: hasLive ? String(live.statementId || "") : String(snapshot.statementId || ""),
                actorId: hasLive ? String(live.actorId || "") : "",
                allocatable: hasLive ? !!live.allocatable : !!snapshot.allocatable,
                description: hasLive ? String(live.description || "") : "",
                propertyIds: hasLive && live.propertyIds ? live.propertyIds : []
            })
        }

        const allTransactions = root.workspaceFacade ? (root.workspaceFacade.transactions() || []) : []
        for (let i = 0; i < allTransactions.length; ++i) {
            const tx = allTransactions[i] || ({})
            const txId = String(tx.id || "")
            if (txId.length === 0 || snapshotById[txId])
                continue
            const year = root.bookingYear(tx.bookingDate)
            if (targetYear > 0 && year === targetYear)
                issues.missingFromYear += 1
        }

        root.annualVerificationIssues = issues
        root.annualStatusMetrics = metrics
        root.annualTransactions = rows
    }

    function normalizedAnalysisDisplay(row) {
        const candidate = row || ({})
        const name = candidate.name !== undefined ? String(candidate.name) : ""
        if (name.length > 0)
            return name
        const id = candidate.id !== undefined ? String(candidate.id) : ""
        return id
    }

    function resolveAssignedAnalyses(allAnalysisRows, ids) {
        const rows = allAnalysisRows || []
        const selectedIds = ids || []
        const out = []

        for (let i = 0; i < selectedIds.length; ++i) {
            const id = String(selectedIds[i] || "")
            if (id.length === 0)
                continue

            for (let j = 0; j < rows.length; ++j) {
                const row = rows[j] || ({})
                if (String(row.id || "") === id) {
                    const normalized = ({})
                    for (const key in row)
                        normalized[key] = row[key]
                    normalized.display = root.normalizedAnalysisDisplay(row)
                    out.push(normalized)
                    break
                }
            }
        }

        return out
    }

    function availableAnalysisRows() {
        const rows = root.analysisRows() || []
        const selectedIds = root.analysisIds || []
        const out = []

        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i] || ({})
            const id = row.id ? String(row.id) : ""
            if (id.length === 0 || selectedIds.indexOf(id) !== -1)
                continue

            const normalized = ({})
            for (const key in row)
                normalized[key] = row[key]
            normalized.id = id
            normalized.display = root.normalizedAnalysisDisplay(row)
            out.push(normalized)
        }

        return out
    }

    function syncFields() {
        if (!root.isEdit) {
            root.clearFields()
            return
        }

        if (!root.workspaceFacade || !root.current || !root.current.id) {
            root.clearFields()
            return
        }

        const payload = root.workspaceFacade.annual(root.current.id)
        root.annualNameText = payload && payload.name !== undefined ? String(payload.name) : ""
        root.yearText = payload && payload.year !== undefined ? String(payload.year) : ""
        root.analysisIds = payload && payload.analysisIds ? payload.analysisIds : []
        root.assignedAnalysisRows = root.resolveAssignedAnalyses(root.analysisRows(), root.analysisIds)
        root.rebuildAnnualDerivedState()
        root.captureSavedState()
    }

    function navigateAnnual(delta) {
        const rows = root.annualRows()
        if (!root.session || rows.length === 0)
            return

        const currentId = root.isEdit ? (root.session.selectedAnnualId || "") : ""
        const fallbackIndex = delta > 0 ? 0 : rows.length - 1
        const nextId = root.session.navigatedId(rows, currentId, delta, fallbackIndex)
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

        const currentId = root.isEdit && root.current && root.current.id ? root.current.id : ""
        const annualId = root.workspaceFacade.saveAnnual(currentId,
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
        if (!root.workspaceFacade || !root.current || !root.current.id)
            return

        const removedId = root.current.id
        root.workspaceFacade.deleteAnnual(removedId)
        if (!root.session) {
            root.clearFields()
            return
        }

        const nextId = root.session.deleteNextSelectionId(root.annualRows(), removedId, 0, "id")
        root.session.selectedAnnualId = nextId || ""
        if (!nextId || nextId.length === 0)
            root.clearFields()
    }

    onCurrentChanged: root.syncFields()
    onIsEditChanged: root.syncFields()

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

                    Controls.TextField {
                        objectName: "annualYearField"
                        Layout.fillWidth: true
                        text: root.yearText
                        inputMethodHints: Qt.ImhDigitsOnly
                        validator: IntValidator { bottom: 1900; top: 2500 }
                        onTextEdited: root.yearText = text
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
                        appContext: root.appContext
                        theme: root.theme
                        allAnalysisRows: root.availableAnalysisRows()
                        analysisRows: root.assignedAnalysisRows
                        selectedAnalysisIds: root.analysisIds
                        onSelectionChanged: function(ids) {
                            root.analysisIds = ids || []
                            root.assignedAnalysisRows = root.resolveAssignedAnalyses(root.analysisRows(), root.analysisIds)
                            root.rebuildAnnualDerivedState()
                        }
                    }

                    Views.AnnualTransactionsPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        theme: root.theme
                        transactions: root.annualTransactions
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
