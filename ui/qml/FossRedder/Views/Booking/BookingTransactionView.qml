/**
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionView.qml
 * @brief Provides the BookingTransactionView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    objectName: "bookingTransactionViewRoot"
    required property var theme
    required property var sessionState
    property var transactionData: ({})
    property var actorRows: []
    property var contractRows: []
    property var propertyRows: []
    signal transactionEdited(var data)

    readonly property var statusOptions: [
        { label: qsTr("Neutral"), value: 0 },
        { label: qsTr("Unverified"), value: 1 },
        { label: qsTr("Verified"), value: 2 },
        { label: qsTr("Completed"), value: 3 }
    ]

    function updateField(key, value) {
        const base = root.transactionData || ({})
        const next = root.sessionState ? root.sessionState.mapWithKeyValue(base, key, value) : base
        root.transactionEdited(next)
    }

    function updateFields(values) {
        const entries = values || ({})
        let next = root.transactionData || ({})
        for (const key in entries)
            next = root.sessionState ? root.sessionState.mapWithKeyValue(next, key, entries[key]) : next
        root.transactionEdited(next)
    }

    function valueOrEmpty(key) {
        return root.transactionData && root.transactionData[key] ? root.transactionData[key] : ""
    }

    function normalizedDraft() {
        return root.sessionState ? root.sessionState.normalizeTransactionDraft(root.transactionData || ({})) : ({})
    }

    function actorIdValue() {
        return root.valueOrEmpty("actorId")
    }

    function contractIdValue() {
        return root.valueOrEmpty("contractId")
    }

    function allocatableValue() {
        return root.transactionData && root.transactionData.allocatable ? true : false
    }

    function selectedPropertyIds() {
        const tx = root.normalizedDraft()
        return tx.propertyIds ? tx.propertyIds : []
    }

    function hasProperty(propertyId) {
        if (!root.sessionState || !propertyId)
            return false
        return root.sessionState.indexOfString(root.selectedPropertyIds(), String(propertyId)) !== -1
    }

    function toggleProperty(propertyId, checked) {
        if (!root.sessionState || !propertyId)
            return

        const key = String(propertyId)
        const ids = checked
            ? root.sessionState.addUniqueTrimmed(root.selectedPropertyIds(), key)
            : root.sessionState.removeString(root.selectedPropertyIds(), key)

        root.applyPropertySelection(ids)
    }

    function actorDisplayModel() {
        return root.sessionState
            ? root.sessionState.displayRowsWithEmpty(root.actorRows || [], qsTr("No actor"), "display")
            : []
    }

    function contractDisplayModel() {
        return root.sessionState
            ? root.sessionState.displayRowsWithEmpty(root.contractRows || [], qsTr("No contract"), "display")
            : []
    }

    function selectedIndexFor(model, id) {
        if (!root.sessionState)
            return 0
        const idx = root.sessionState.indexOfId(model || [], String(id || ""))
        return idx >= 0 ? idx : 0
    }

    function applyContractSelection(contractId) {
        if (!root.sessionState || !root.sessionState.transactionDraft)
            return
        const selectedContractId = String(contractId || "").trim()
        const next = root.sessionState.transactionDraft(
            root.transactionData || ({}),
            root.contractRows || [],
            { contractId: selectedContractId })
        root.transactionEdited(next || ({}))
    }

    function applyActorSelection(actorId) {
        if (!root.sessionState || !root.sessionState.transactionDraft)
            return
        const selectedActorId = String(actorId || "").trim()
        const next = root.sessionState.transactionDraft(
            root.transactionData || ({}),
            root.contractRows || [],
            { actorId: selectedActorId })
        root.transactionEdited(next || ({}))
    }

    function applyPropertySelection(propertyIds) {
        if (!root.sessionState || !root.sessionState.transactionDraft)
            return
        const ids = propertyIds ? propertyIds.slice() : []
        const next = root.sessionState.transactionDraft(
            root.transactionData || ({}),
            root.contractRows || [],
            { propertyIds: ids })
        root.transactionEdited(next || ({}))
    }

    function statusIndex() {
        if (!root.sessionState)
            return 0
        const tx = root.normalizedDraft()
        const idx = root.sessionState.indexOfKeyValue(root.statusOptions, "value", tx.status !== undefined ? tx.status : 0)
        return idx >= 0 ? idx : 0
    }

    implicitHeight: txLayout.implicitHeight

    ColumnLayout {
        id: txLayout
        width: parent ? parent.width : 0
        spacing: root.theme.spacingSmall

        Views.BookingTransactionFieldRow {
            theme: root.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Name")
            rightLabel: qsTr("Status")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    objectName: "bookingTransactionNameField"
                    text: root.valueOrEmpty("name")
                    onTextEdited: root.updateField("name", text)
                }
            }

            rightContent: Component {
                Controls.DropdownMenu {
                    objectName: "bookingTransactionStatusComboBox"
                    textRole: "label"
                    model: root.statusOptions
                    currentIndex: root.statusIndex()
                    onActivated: function(index) { root.updateField("status", root.statusOptions[index].value) }
                }
            }
        }

        Views.BookingTransactionFieldRow {
            theme: root.theme
            Layout.fillWidth: true
            leftLabel: qsTr("Booking Date")
            rightLabel: qsTr("Valuta")
            leftWeight: 3
            rightWeight: 2

            leftContent: Component {
                Controls.TextField {
                    objectName: "bookingTransactionBookingDateField"
                    text: root.valueOrEmpty("bookingDate")
                    onTextEdited: root.updateField("bookingDate", text)
                }
            }

            rightContent: Component {
                Controls.TextField {
                    objectName: "bookingTransactionValutaField"
                    text: root.valueOrEmpty("valuta")
                    onTextEdited: root.updateField("valuta", text)
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingSmall

            Label {
                text: qsTr("Amount")
                Layout.fillWidth: true
            }

            Controls.TextField {
                objectName: "bookingTransactionAmountField"
                Layout.fillWidth: true
                text: root.transactionData && root.transactionData.amount !== undefined ? String(root.transactionData.amount) : ""
                onTextEdited: root.updateField("amount", text)
            }
        }

        Views.BookingTransactionActorPanel {
            theme: root.theme
            txRoot: root
        }

        Views.BookingTransactionContractPanel {
            theme: root.theme
            txRoot: root
        }

        Views.BookingTransactionPropertyPanel {
            theme: root.theme
            txRoot: root
        }

        Views.BookingTransactionAllocatablePanel {
            theme: root.theme
            txRoot: root
        }
    }
}
