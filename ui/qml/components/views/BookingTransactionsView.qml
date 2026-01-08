import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    property bool isNew: uiData && uiData.selectedTransactionId === "__new__"
    property var current: (!isNew && uiData) ? uiData.selectedTransaction : null
    property bool isEdit: !isNew && current && current.id && String(current.id).length > 0

    function clearFields() {
        nameField.text = ""
        bookingDateField.text = ""
        amountField.text = ""
        descField.text = ""
        statementIdField.text = (uiData && uiData.selectedStatementId) ? uiData.selectedStatementId : ""
        quickStatementField.text = ""
    }

    function syncFields() {
        if (isNew) {
            clearFields()
            return
        }
        if (!isEdit) {
            clearFields()
            return
        }
        nameField.text = current.name || ""
        bookingDateField.text = current.bookingDate || ""
        amountField.text = String(current.amount)
        descField.text = current.description || ""
        statementIdField.text = current.statementId || ""
        allocCheck.checked = current.allocatable ? true : false
        statusCombo.currentIndex = Math.max(0, current.status)
    }

    function toFileUrl(p) {
        if (!p || p.length === 0) return ""
        if (p.indexOf("file://") === 0) return p
        return "file:///" + String(p).replace(/\\/g, "/")
    }

    Connections { target: current; function onChanged() { syncFields() } }
    onIsNewChanged: syncFields()
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label { text: (isNew || isEdit) ? qsTr("Transaction") : qsTr("Create Transaction"); font.pointSize: 18 }

        TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }
        TextField { id: bookingDateField; placeholderText: qsTr("Booking Date"); Layout.fillWidth: true }
        TextField { id: amountField; placeholderText: qsTr("Amount"); Layout.fillWidth: true }

        TextArea { id: descField; placeholderText: qsTr("Description"); Layout.fillWidth: true; Layout.preferredHeight: 120; wrapMode: TextArea.Wrap }

        GroupBox {
            title: qsTr("Statement (required)")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                ComboBox {
                    id: statementCombo
                    Layout.fillWidth: true
                    model: uiData ? uiData.statements : null
                    textRole: "name"
                    valueRole: "id"
                    onActivated: {
                        statementIdField.text = currentValue
                    }
                    Component.onCompleted: {
                        if (statementIdField.text.length > 0) {
                            for (var i = 0; i < count; ++i) {
                                if (valueAt(i) === statementIdField.text) {
                                    currentIndex = i
                                    break
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    TextField { id: quickStatementField; placeholderText: qsTr("New statement name"); Layout.fillWidth: true }
                    Button {
                        text: qsTr("Add")
                        enabled: quickStatementField.text.trim().length > 0
                        onClicked: {
                            if (!uiDomain) return
                            var id = uiDomain.ensureStatementByName(quickStatementField.text)
                            if (id && id.length > 0) {
                                statementIdField.text = id
                                if (uiData && (!uiData.selectedStatementId || uiData.selectedStatementId === "")) uiData.selectedStatementId = id
                            }
                            quickStatementField.text = ""
                        }
                    }
                }

                TextField {
                    id: statementIdField
                    visible: false
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                id: allocCheck
                text: qsTr("Allocatable to tenant")
                checked: false
                onCheckedChanged: {
                    // when editing existing transaction, update persistent model
                    if (!isNew && uiDomain && current && current.id) {
                        uiDomain.updateTransactionAllocatable(current.id, checked)
                    }
                }
            }

            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            ComboBox {
                id: statusCombo
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: 2
                onActivated: {
                    if (!isNew && uiDomain && current && current.id) {
                        uiDomain.updateTransactionStatus(current.id, currentIndex)
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Back")
                onClicked: {
                    if (uiData) uiData.selectedTransactionId = ""
                }
            }

            Button {
                text: (isEdit ? qsTr("Update") : qsTr("Add"))
                enabled: nameField.text.length > 0 && statementIdField.text.length > 0
                onClicked: {
                    if (!uiDomain) return
                    var amount = parseFloat(amountField.text)
                    if (isNaN(amount)) amount = 0.0

                    if (isEdit) {
                        uiDomain.updateTransaction(current.id,
                                                  nameField.text,
                                                  bookingDateField.text,
                                                  amount,
                                                  descField.text,
                                                  statementIdField.text)
                        // apply allocatable/status changes for existing transaction
                        uiDomain.updateTransactionAllocatable(current.id, allocCheck.checked)
                        uiDomain.updateTransactionStatus(current.id, statusCombo.currentIndex)
                     } else {
                        var id = uiDomain.addTransaction(nameField.text,
                                                         bookingDateField.text,
                                                         amount,
                                                         descField.text,
                                                         statementIdField.text)
                         // after creation, set allocatable and status if needed
                         if (id && id.length > 0) {
                             uiDomain.updateTransactionAllocatable(id, allocCheck.checked)
                             uiDomain.updateTransactionStatus(id, statusCombo.currentIndex)
                         }
                         clearFields()
                         if (uiData && id && id.length > 0) uiData.selectedTransactionId = id
                     }
                }
            }

            Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!uiDomain) return
                    uiDomain.deleteTransaction(current.id)
                    if (uiData) uiData.selectedTransactionId = ""
                    clearFields()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: syncFields()
}
