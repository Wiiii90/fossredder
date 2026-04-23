/*!
 * @file ui/qml/FossRedder/Views/Booking/BookingTransactionView.qml
 * @brief Form view for a single transaction inside the booking statement page.
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
        let next = {}
        if (root.transactionData) {
            for (const k in root.transactionData)
                next[k] = root.transactionData[k]
        }
        next[key] = value
        root.transactionEdited(next)
    }

    function valueOrEmpty(key) {
        return root.transactionData && root.transactionData[key] ? root.transactionData[key] : ""
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

    function toAmount(value) {
        const parsed = parseFloat(value)
        return isNaN(parsed) ? 0.0 : parsed
    }

    function selectedPropertyIds() {
        return root.transactionData && root.transactionData.propertyIds ? root.transactionData.propertyIds.slice() : []
    }

    function hasProperty(propertyId) {
        return propertyId && root.selectedPropertyIds().indexOf(propertyId) !== -1
    }

    function toggleProperty(propertyId, checked) {
        if (!propertyId)
            return

        let ids = root.selectedPropertyIds()
        const idx = ids.indexOf(propertyId)
        if (checked && idx === -1)
            ids.push(propertyId)
        else if (!checked && idx !== -1)
            ids.splice(idx, 1)

        root.updateField("propertyIds", ids)
    }

    function actorDisplayModel() {
        const out = []
        out.push({ id: "", display: qsTr("No actor") })
        const rows = root.actorRows || []
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            out.push({
                id: row && row.id ? row.id : "",
                display: row && row.display ? row.display : (row && row.name ? row.name : "")
            })
        }
        return out
    }

    function contractDisplayModel() {
        const out = []
        out.push({ id: "", display: qsTr("No contract") })
        const rows = root.contractRows || []
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            const name = row && row.name ? row.name : ""
            const type = row && row.type ? row.type : ""
            const display = (name.length > 0 && type.length > 0) ? (name + " (" + type + ")") : (row && row.display ? row.display : name)
            out.push({
                id: row && row.id ? row.id : "",
                display: display
            })
        }
        return out
    }

    function selectedIndexFor(model, id) {
        const rows = model || []
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i].id === id)
                return i
        }
        return 0
    }

    function statusIndex() {
        const current = root.transactionData && root.transactionData.status !== undefined ? Number(root.transactionData.status) : 0
        for (let i = 0; i < root.statusOptions.length; ++i) {
            if (Number(root.statusOptions[i].value) === current)
                return i
        }
        return 0
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
                    text: root.valueOrEmpty("name")
                    onTextEdited: root.updateField("name", text)
                }
            }

            rightContent: Component {
                Controls.ComboBox {
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
                    text: root.valueOrEmpty("bookingDate")
                    onTextEdited: root.updateField("bookingDate", text)
                }
            }

            rightContent: Component {
                Controls.TextField {
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
                Layout.fillWidth: true
                text: root.transactionData && root.transactionData.amount !== undefined ? String(root.transactionData.amount) : ""
                onTextEdited: root.updateField("amount", root.toAmount(text))
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
