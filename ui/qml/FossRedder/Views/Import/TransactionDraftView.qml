/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftView.qml
 * @brief Coordinates transaction draft panels and keeps draft view state synchronized.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: draftViewRoot
    required property var appContext
    required property var theme

    readonly property var importWorkflow: draftViewRoot.appContext ? draftViewRoot.appContext.importWorkflow : null

    property var draft
    property var viewState: ({})
    property bool syncScheduled: false
    property string pendingNameText: ""
    property string pendingBookingDateText: ""
    property string pendingValutaText: ""
    readonly property var actorChoices: draftViewRoot.viewState && draftViewRoot.viewState.actorChoices ? draftViewRoot.viewState.actorChoices : []
    readonly property var statusOptions: [
        { label: qsTr("Neutral"), value: 0 },
        { label: qsTr("Unverified"), value: 1 },
        { label: qsTr("Verified"), value: 2 },
        { label: qsTr("Completed"), value: 3 }
    ]

    function currentDraftItem() {
        return draftViewRoot.draft && draftViewRoot.draft.current ? draftViewRoot.draft.current : null
    }

    function currentTransaction() {
        return draftViewRoot.currentDraftItem()
    }

    function syncAmountFieldToCurrent() {
        const tx = draftViewRoot.currentTransaction()
        const nextText = tx && tx.amount !== undefined ? String(tx.amount) : ""
        if (amountField.text !== nextText) amountField.text = nextText
    }

    function syncPendingTextsToCurrent() {
        const tx = draftViewRoot.currentTransaction()
        draftViewRoot.pendingNameText = tx && tx.name !== undefined && tx.name !== null ? String(tx.name) : ""
        draftViewRoot.pendingBookingDateText = tx && tx.bookingDate !== undefined && tx.bookingDate !== null ? String(tx.bookingDate) : ""
        draftViewRoot.pendingValutaText = tx && tx.valuta !== undefined && tx.valuta !== null ? String(tx.valuta) : ""
    }

    function suggestionBucket(kind) {
        const tx = draftViewRoot.currentTransaction()
        return tx && tx[kind] ? tx[kind] : ({ candidates: [] })
    }

    function topSuggestion(bucket) {
        return bucket && bucket.candidates && bucket.candidates.length > 0 ? bucket.candidates[0] : ({})
    }

    function currentStatusValue() {
        return draftViewRoot.draft && draftViewRoot.draft.current && draftViewRoot.draft.current.status !== undefined ? Number(draftViewRoot.draft.current.status) : 0
    }

    function actorTopSuggestion() { return draftViewRoot.topSuggestion(draftViewRoot.suggestionBucket("actorSuggestions")) }
    function propertyTopSuggestion() { return draftViewRoot.topSuggestion(draftViewRoot.suggestionBucket("propertySuggestions")) }
    function contractTopSuggestion() { return draftViewRoot.topSuggestion(draftViewRoot.suggestionBucket("contractSuggestions")) }

    function suggestionConfidencePercent(s) {
        const confidence = s && s.confidence !== undefined ? Number(s.confidence) : 0.0
        return Math.round(confidence * 100.0)
    }

    function suggestionColor(s) {
        const confidence = s && s.confidence !== undefined ? Number(s.confidence) : 0.0
        if (confidence >= 0.9) return draftViewRoot.theme.successStrong
        if (confidence >= 0.4) return draftViewRoot.theme.warning
        return draftViewRoot.theme.danger
    }

    function statusCurrentIndex() {
        const value = draftViewRoot.currentStatusValue()
        for (let i = 0; i < draftViewRoot.statusOptions.length; ++i) {
            if (Number(draftViewRoot.statusOptions[i].value) === value) return i
        }
        return 0
    }

    function setStatusByIndex(index) {
        if (!draftViewRoot.draft || index < 0 || index >= draftViewRoot.statusOptions.length) return
        draftViewRoot.draft.transactions.setStatus(draftViewRoot.draft.currentIndex, draftViewRoot.statusOptions[index].value)
        if (draftViewRoot.draft.refresh) draftViewRoot.draft.refresh()
    }

    function propertySuggestionSummary() {
        const bucket = draftViewRoot.suggestionBucket("propertySuggestions")
        if (!bucket || !bucket.candidates || bucket.candidates.length === 0) return qsTr("No property suggestion")

        const labels = []
        const maxCount = Math.min(2, bucket.candidates.length)
        for (let i = 0; i < maxCount; ++i) {
            if (bucket.candidates[i] && bucket.candidates[i].label) labels.push(bucket.candidates[i].label)
        }

        const summarySuggestion = draftViewRoot.topSuggestion(bucket)
        return qsTr("Confidence: %1% (%2)").arg(draftViewRoot.suggestionConfidencePercent(summarySuggestion)).arg(labels.join(", "))
    }

    function effectiveAllocatable() {
        const tx = draftViewRoot.currentTransaction()
        if (!tx) return false
        if (tx.allocatableSelected) return !!tx.allocatable
        return draftViewRoot.viewState && draftViewRoot.viewState.effectiveAllocatable !== undefined ? !!draftViewRoot.viewState.effectiveAllocatable : !!tx.allocatable
    }

    function proofSource(proofDataBase64) {
        const data = proofDataBase64 || ""
        if (data.length === 0) return ""
        return "data:image/jpeg;base64," + data
    }

    function syncViewState() {
        draftViewRoot.syncScheduled = false
        if (!(draftViewRoot.importWorkflow && draftViewRoot.draft)) {
            draftViewRoot.viewState = ({})
            return
        }

        draftViewRoot.viewState = draftViewRoot.importWorkflow.currentTransactionViewState(draftViewRoot.draft)
    }

    function scheduleSyncViewState() {
        if (draftViewRoot.syncScheduled) return
        draftViewRoot.syncScheduled = true
        Qt.callLater(function() {
            draftViewRoot.syncViewState()
        })
    }

    function refreshDerivedState() {
        draftViewRoot.scheduleSyncViewState()
    }

    function commitNameText(value) {
        if (!draftViewRoot.draft) return
        const tx = draftViewRoot.currentTransaction()
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = tx && tx.name !== undefined && tx.name !== null ? String(tx.name) : ""
        if (nextValue !== currentValue)
            draftViewRoot.draft.transactions.setName(draftViewRoot.draft.currentIndex, nextValue)
        draftViewRoot.refreshDerivedState()
    }

    function commitBookingDateText(value) {
        if (!draftViewRoot.draft) return
        const tx = draftViewRoot.currentTransaction()
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = tx && tx.bookingDate !== undefined && tx.bookingDate !== null ? String(tx.bookingDate) : ""
        if (nextValue !== currentValue)
            draftViewRoot.draft.transactions.setBookingDate(draftViewRoot.draft.currentIndex, nextValue)
        draftViewRoot.refreshDerivedState()
    }

    function commitValutaText(value) {
        if (!draftViewRoot.draft) return
        const tx = draftViewRoot.currentTransaction()
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = tx && tx.valuta !== undefined && tx.valuta !== null ? String(tx.valuta) : ""
        if (nextValue !== currentValue)
            draftViewRoot.draft.transactions.setValuta(draftViewRoot.draft.currentIndex, nextValue)
        draftViewRoot.refreshDerivedState()
    }

    function commitAmountText(value) {
        if (draftViewRoot.importWorkflow && draftViewRoot.draft && draftViewRoot.importWorkflow.updateCurrentAmount)
            draftViewRoot.importWorkflow.updateCurrentAmount(draftViewRoot.draft, value)
        draftViewRoot.refreshDerivedState()
    }

    function commitPendingEdits() {
        draftViewRoot.commitNameText(draftViewRoot.pendingNameText)
        draftViewRoot.commitBookingDateText(draftViewRoot.pendingBookingDateText)
        draftViewRoot.commitValutaText(draftViewRoot.pendingValutaText)
        draftViewRoot.commitAmountText(amountField.text)
        if (metadataPanel && metadataPanel.commitMetadata)
            metadataPanel.commitMetadata(metadataPanel.currentText())
        if (actorPanel && actorPanel.commitActorText)
            actorPanel.commitActorText(actorPanel.currentText())
        if (contractPanel && contractPanel.commitTypeText)
            contractPanel.commitTypeText(contractPanel.currentTypeText())
    }

    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth
    height: draftViewRoot.implicitHeight

    onDraftChanged: {
        draftViewRoot.syncPendingTextsToCurrent()
        draftViewRoot.syncAmountFieldToCurrent()
        draftViewRoot.scheduleSyncViewState()
    }

    Connections {
        target: draftViewRoot.draft
        function onChanged() {}
        function onCurrentIndexChanged() {
            draftViewRoot.syncPendingTextsToCurrent()
            draftViewRoot.syncAmountFieldToCurrent()
            draftViewRoot.scheduleSyncViewState()
        }
        function onCurrentChanged() {
            draftViewRoot.syncPendingTextsToCurrent()
            draftViewRoot.syncAmountFieldToCurrent()
            draftViewRoot.scheduleSyncViewState()
        }
        function onCountChanged() {
            draftViewRoot.syncPendingTextsToCurrent()
            draftViewRoot.syncAmountFieldToCurrent()
            draftViewRoot.scheduleSyncViewState()
        }
    }

    Component.onCompleted: {
        draftViewRoot.syncPendingTextsToCurrent()
        draftViewRoot.syncAmountFieldToCurrent()
        draftViewRoot.scheduleSyncViewState()
    }

    ColumnLayout {
        id: txLayout
        width: draftViewRoot.width
        spacing: draftViewRoot.theme.spacingSmall

        TransactionDraftFieldRow {
            theme: draftViewRoot.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Name")
            rightLabel: qsTr("Status")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField { id: nameField
                    objectName: "transactionDraftNameField"
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.name || "") : ""
                    onTextEdited: draftViewRoot.pendingNameText = nameField.text
                    onEditingFinished: draftViewRoot.commitNameText(nameField.text)
                    onAccepted: draftViewRoot.commitNameText(nameField.text)
                    onActiveFocusChanged: if (!activeFocus) draftViewRoot.commitNameText(nameField.text)
                }
            }

            rightContent: Component {
                Controls.DropdownMenu { id: statusCombo
                    textRole: "label"
                    model: draftViewRoot.statusOptions
                    currentIndex: draftViewRoot.statusCurrentIndex()
                    onActivated: function(index) { draftViewRoot.setStatusByIndex(index) }
                }
            }
        }

        TransactionDraftProofPanel { txRoot: draftViewRoot }

        TransactionDraftFieldRow {
            theme: draftViewRoot.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Booking Date")
            rightLabel: qsTr("Valuta")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField { id: bookingDateField
                    objectName: "transactionDraftBookingDateField"
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.bookingDate || "") : ""
                    onTextEdited: draftViewRoot.pendingBookingDateText = bookingDateField.text
                    onEditingFinished: draftViewRoot.commitBookingDateText(bookingDateField.text)
                    onAccepted: draftViewRoot.commitBookingDateText(bookingDateField.text)
                    onActiveFocusChanged: if (!activeFocus) draftViewRoot.commitBookingDateText(bookingDateField.text)
                }
            }

            rightContent: Component {
                Controls.TextField { id: valutaField
                    objectName: "transactionDraftValutaField"
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.valuta || "") : ""
                    onTextEdited: draftViewRoot.pendingValutaText = valutaField.text
                    onEditingFinished: draftViewRoot.commitValutaText(valutaField.text)
                    onAccepted: draftViewRoot.commitValutaText(valutaField.text)
                    onActiveFocusChanged: if (!activeFocus) draftViewRoot.commitValutaText(valutaField.text)
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: draftViewRoot.theme.spacingSmall

            Label {
                text: qsTr("Amount")
                Layout.fillWidth: true
            }

            Controls.TextField {
                id: amountField
                objectName: "transactionDraftAmountField"
                Layout.fillWidth: true
                text: draftViewRoot.draft && draftViewRoot.draft.current && draftViewRoot.draft.current.amount !== undefined
                      ? String(draftViewRoot.draft.current.amount)
                      : ""
                onEditingFinished: draftViewRoot.commitAmountText(amountField.text)
                onAccepted: draftViewRoot.commitAmountText(amountField.text)
                onActiveFocusChanged: if (!activeFocus) draftViewRoot.commitAmountText(amountField.text)
            }
        }

        TransactionDraftMetadataPanel { id: metadataPanel; txRoot: draftViewRoot }

        TransactionDraftActorPanel { id: actorPanel; txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }
        TransactionDraftContractPanel { id: contractPanel; txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }
        TransactionDraftPropertyPanel { txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }

        TransactionDraftAllocatablePanel { txRoot: draftViewRoot }
    }

}
