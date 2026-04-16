import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: draftViewRoot
    required property var appContext
    required property var theme

    readonly property var draftController: draftViewRoot.appContext ? draftViewRoot.appContext.draftController : null

    property var draft
    property var viewState: ({})
    property bool syncScheduled: false
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
        if (tx.allocatableManualOverride) return !!tx.allocatable
        return draftViewRoot.viewState && draftViewRoot.viewState.effectiveAllocatable !== undefined ? !!draftViewRoot.viewState.effectiveAllocatable : !!tx.allocatable
    }

    function proofSource(proofDataBase64) {
        const data = proofDataBase64 || ""
        if (data.length === 0) return ""
        return "data:image/jpeg;base64," + data
    }

    function syncViewState() {
        draftViewRoot.syncScheduled = false
        if (!(draftViewRoot.draftController && draftViewRoot.draft)) {
            draftViewRoot.viewState = ({})
            return
        }

        draftViewRoot.draftController.syncCurrentTransactionDraft(draftViewRoot.draft)
        draftViewRoot.viewState = draftViewRoot.draftController.currentTransactionViewState(draftViewRoot.draft)
    }

    function scheduleSyncViewState() {
        if (draftViewRoot.syncScheduled) return
        draftViewRoot.syncScheduled = true
        Qt.callLater(function() {
            draftViewRoot.syncViewState()
        })
    }

    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth
    height: draftViewRoot.implicitHeight

    onDraftChanged: draftViewRoot.scheduleSyncViewState()

    Connections {
        target: draftViewRoot.draft
        function onChanged() { draftViewRoot.scheduleSyncViewState() }
    }

    Component.onCompleted: draftViewRoot.scheduleSyncViewState()

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
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.name || "") : ""
                    onTextEdited: if (draftViewRoot.draft) draftViewRoot.draft.transactions.setName(draftViewRoot.draft.currentIndex, nameField.text)
                }
            }

            rightContent: Component {
                Controls.ComboBox { id: statusCombo
                    textRole: "label"
                    model: draftViewRoot.statusOptions
                    currentIndex: draftViewRoot.statusCurrentIndex()
                    onActivated: draftViewRoot.setStatusByIndex(statusCombo.currentIndex)
                }
            }
        }

        TransactionDraftProofPanel { txRoot: draftViewRoot }

        TransactionDraftFieldRow {
            theme: draftViewRoot.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Buchungsdatum")
            rightLabel: qsTr("Valuta")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField { id: bookingDateField
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.bookingDate || "") : ""
                    onTextEdited: if (draftViewRoot.draft) draftViewRoot.draft.transactions.setBookingDate(draftViewRoot.draft.currentIndex, bookingDateField.text)
                }
            }

            rightContent: Component {
                Controls.TextField { id: valutaField
                    text: draftViewRoot.draft && draftViewRoot.draft.current ? (draftViewRoot.draft.current.valuta || "") : ""
                    onTextEdited: if (draftViewRoot.draft) draftViewRoot.draft.transactions.setValuta(draftViewRoot.draft.currentIndex, valutaField.text)
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
                Layout.fillWidth: true
                text: draftViewRoot.draft && draftViewRoot.draft.current ? String(draftViewRoot.draft.current.amount) : ""
                onEditingFinished: {
                    if (draftViewRoot.draftController && draftViewRoot.draft)
                        draftViewRoot.draftController.updateCurrentAmount(draftViewRoot.draft, amountField.text)
                }
                onAccepted: {
                    if (draftViewRoot.draftController && draftViewRoot.draft)
                        draftViewRoot.draftController.updateCurrentAmount(draftViewRoot.draft, amountField.text)
                }
            }
        }

        TransactionDraftMetadataPanel { txRoot: draftViewRoot }

        TransactionDraftActorPanel { txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }
        TransactionDraftContractPanel { txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }
        TransactionDraftPropertyPanel { txRoot: draftViewRoot; appContext: draftViewRoot.appContext; theme: draftViewRoot.theme }

        TransactionDraftAllocatablePanel { txRoot: draftViewRoot }
    }

}
