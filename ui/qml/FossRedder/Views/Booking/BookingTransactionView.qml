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
    required property var theme
    required property var session
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
        const next = root.session ? root.session.mapWithKeyValue(base, key, value) : base
        root.transactionEdited(next)
    }

    function valueOrEmpty(key) {
        return root.transactionData && root.transactionData[key] ? root.transactionData[key] : ""
    }

    function normalizedDraft() {
        return root.session ? root.session.normalizeTransactionDraft(root.transactionData || ({})) : ({})
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
        if (!root.session || !propertyId)
            return false
        return root.session.indexOfString(root.selectedPropertyIds(), String(propertyId)) !== -1
    }

    function toggleProperty(propertyId, checked) {
        if (!root.session || !propertyId)
            return

        const key = String(propertyId)
        const ids = checked
            ? root.session.addUniqueTrimmed(root.selectedPropertyIds(), key)
            : root.session.removeString(root.selectedPropertyIds(), key)

        root.updateField("propertyIds", ids)
    }

    function actorDisplayModel() {
        return root.session
            ? root.session.displayRowsWithEmpty(root.actorRows || [], qsTr("No actor"), "display")
            : []
    }

    function contractDisplayModel() {
        return root.session
            ? root.session.displayRowsWithEmpty(root.contractRows || [], qsTr("No contract"), "display")
            : []
    }

    function selectedIndexFor(model, id) {
        if (!root.session)
            return 0
        const idx = root.session.indexOfId(model || [], String(id || ""))
        return idx >= 0 ? idx : 0
    }

    function statusIndex() {
        if (!root.session)
            return 0
        const tx = root.normalizedDraft()
        const idx = root.session.indexOfKeyValue(root.statusOptions, "value", tx.status !== undefined ? tx.status : 0)
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
                    onActivated: root.updateField("status", root.statusOptions[currentIndex].value)
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
                onTextEdited: {
                    const normalized = root.session ? root.session.normalizeTransactionDraft({ amount: text }) : ({ amount: 0.0 })
                    root.updateField("amount", normalized.amount)
                }
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
