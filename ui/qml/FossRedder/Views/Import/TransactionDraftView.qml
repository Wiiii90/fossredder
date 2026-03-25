import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: txRoot

    property var draft
    property var viewState: ({})
    property bool syncScheduled: false
    readonly property var actorChoices: viewState && viewState.actorChoices ? viewState.actorChoices : []
    readonly property var statusOptions: [
        { label: qsTr("Neutral"), value: 0 },
        { label: qsTr("Unverified"), value: 1 },
        { label: qsTr("Verified"), value: 2 },
        { label: qsTr("Completed"), value: 3 }
    ]

    function currentTransaction() {
        return draft && draft.current ? draft.current : null
    }

    function suggestionBucket(kind) {
        var tx = currentTransaction()
        return tx && tx[kind] ? tx[kind] : ({ candidates: [] })
    }

    function topSuggestion(bucket) {
        return bucket && bucket.candidates && bucket.candidates.length > 0 ? bucket.candidates[0] : ({})
    }

    function actorTopSuggestion() { return topSuggestion(suggestionBucket("actorSuggestions")) }
    function propertyTopSuggestion() { return topSuggestion(suggestionBucket("propertySuggestions")) }
    function contractTopSuggestion() { return topSuggestion(suggestionBucket("contractSuggestions")) }

    function suggestionConfidencePercent(s) {
        var confidence = s && s.confidence !== undefined ? Number(s.confidence) : 0.0
        return Math.round(confidence * 100.0)
    }

    function suggestionColor(s) {
        var confidence = s && s.confidence !== undefined ? Number(s.confidence) : 0.0
        if (confidence >= 0.9) return Theme.successStrong
        if (confidence >= 0.4) return Theme.warning
        return Theme.danger
    }

    function statusCurrentIndex() {
        var value = draft && draft.current && draft.current.status !== undefined ? Number(draft.current.status) : 0
        for (var i = 0; i < statusOptions.length; ++i) {
            if (Number(statusOptions[i].value) === value) return i
        }
        return 0
    }

    function setStatusByIndex(index) {
        if (!draft || index < 0 || index >= statusOptions.length) return
        draft.transactions.setStatus(draft.currentIndex, statusOptions[index].value)
        if (draft.refresh) draft.refresh()
    }

    function propertySuggestionSummary() {
        var bucket = suggestionBucket("propertySuggestions")
        if (!bucket || !bucket.candidates || bucket.candidates.length === 0) return qsTr("No property suggestion")

        var labels = []
        var maxCount = Math.min(2, bucket.candidates.length)
        for (var i = 0; i < maxCount; ++i) {
            if (bucket.candidates[i] && bucket.candidates[i].label) labels.push(bucket.candidates[i].label)
        }

        return qsTr("Confidence: %1% (%2)").arg(suggestionConfidencePercent(topSuggestion(bucket))).arg(labels.join(", "))
    }

    function effectiveAllocatable() {
        var tx = currentTransaction()
        if (!tx) return false
        if (tx.allocatableManualOverride) return !!tx.allocatable
        return viewState && viewState.effectiveAllocatable !== undefined ? !!viewState.effectiveAllocatable : !!tx.allocatable
    }

    function proofSource(path) {
        var sourcePath = path || ""
        if (sourcePath.length === 0) return ""
        if (sourcePath.indexOf("proof/") === 0) return "image://importProof/" + sourcePath
        if (sourcePath.indexOf("file://") === 0) return sourcePath

        var normalized = String(sourcePath).replace(/\\/g, "/")
        if (/^[A-Za-z]:\//.test(normalized)) return "file:///" + normalized
        if (normalized.indexOf("//") === 0) return "file:" + normalized
        return "file:///" + normalized
    }

    function syncViewState() {
        syncScheduled = false
        if (!(draftController && draft)) {
            viewState = ({})
            return
        }

        draftController.syncCurrentTransactionDraft(draft)
        viewState = draftController.currentTransactionViewState(draft)
    }

    function scheduleSyncViewState() {
        if (syncScheduled) return
        syncScheduled = true
        Qt.callLater(function() {
            txRoot.syncViewState()
        })
    }

    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth
    height: implicitHeight

    onDraftChanged: scheduleSyncViewState()

    Connections {
        target: draft
        function onChanged() { txRoot.scheduleSyncViewState() }
    }

    Component.onCompleted: scheduleSyncViewState()

    ColumnLayout {
        id: txLayout
        width: txRoot.width
        spacing: Theme.spacingSmall

        TransactionDraftFieldRow {
            Layout.fillWidth: true
            leftLabel: qsTr("Name")
            rightLabel: qsTr("Status")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    text: draft && draft.current ? (draft.current.name || "") : ""
                    onTextEdited: if (draft) draft.transactions.setName(draft.currentIndex, text)
                }
            }

            rightContent: Component {
                Controls.ComboBox {
                    textRole: "label"
                    model: txRoot.statusOptions
                    currentIndex: txRoot.statusCurrentIndex()
                    onActivated: txRoot.setStatusByIndex(currentIndex)
                }
            }
        }

        TransactionDraftProofPanel { txRoot: txRoot }

        TransactionDraftFieldRow {
            Layout.fillWidth: true
            leftLabel: qsTr("Buchungsdatum")
            rightLabel: qsTr("Valuta")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    text: draft && draft.current ? (draft.current.bookingDate || "") : ""
                    onTextEdited: if (draft) draft.transactions.setBookingDate(draft.currentIndex, text)
                }
            }

            rightContent: Component {
                Controls.TextField {
                    text: draft && draft.current ? (draft.current.valuta || "") : ""
                    onTextEdited: if (draft) draft.transactions.setValuta(draft.currentIndex, text)
                }
            }
        }

        TransactionDraftMetadataPanel { txRoot: txRoot }

        TransactionDraftActorPanel { txRoot: txRoot }
        TransactionDraftContractPanel { txRoot: txRoot }
        TransactionDraftPropertyPanel { txRoot: txRoot }

        TransactionDraftAllocatablePanel { txRoot: txRoot }
    }

}
