import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    property bool isNew: uiData && uiData.selectedTransactionId === "__new__"
    property var current: (!isNew && uiData) ? uiData.selectedTransaction : null
    property bool isEdit: !isNew && current && current.id && String(current.id).length > 0

    function clearFields() {
        nameField.text = ""
        bookingDateField.text = ""
        amountField.text = ""
        typeField.text = ""
        if (typeof selectedPropertyIds !== 'undefined') selectedPropertyIds = []
        allocCheck.checked = false
        statusCombo.currentIndex = 0
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
        typeField.text = current.type || ""
        if (typeof selectedPropertyIds !== 'undefined') selectedPropertyIds = current.propertyIds ? current.propertyIds.slice() : []
        allocCheck.checked = current.allocatable ? true : false
        statusCombo.currentIndex = Math.max(0, current.status)
    }

    function effectiveStatementId() {
        return (current && current.statementId && current.statementId.length > 0)
                ? current.statementId
                : ((uiData && uiData.selectedStatementId) ? uiData.selectedStatementId : "")
    }

    function persistCurrentTransaction() {
        if (isNew || !transactionController || !current || !current.id) return
        var amt = parseFloat(amountField.text)
        if (isNaN(amt)) amt = 0.0
        var sid = effectiveStatementId()
        var actorId = (current && current.actorId) ? current.actorId : ""
        var desc = (current && current.description) ? current.description : ""
        transactionController.updateTransaction(current.id,
                                               nameField.text,
                                               bookingDateField.text,
                                               amt,
                                               desc,
                                               sid,
                                               statusCombo.currentIndex,
                                               actorId,
                                               allocCheck.checked,
                                               selectedPropertyIds)
    }

    property var selectedPropertyIds: []

    Connections { target: current; function onChanged() { syncFields() } }
    onIsNewChanged: syncFields()
    onIsEditChanged: syncFields()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label { text: (isNew || isEdit) ? qsTr("Transaction") : qsTr("Create Transaction"); font.pointSize: 18 }

        RowLayout { Layout.fillWidth: true
            Label { text: qsTr("Name"); Layout.preferredWidth: 120 }
            Controls.TextField {
                id: nameField; Layout.fillWidth: true
                onActiveFocusChanged: {
                    if (!activeFocus) persistCurrentTransaction()
                }
                onTextChanged: {
                }
            }
        }

        RowLayout { Layout.fillWidth: true
            Label { text: qsTr("Booking date"); Layout.preferredWidth: 120 }
            Controls.TextField {
                id: bookingDateField; Layout.fillWidth: true
                onActiveFocusChanged: {
                    if (!activeFocus) persistCurrentTransaction()
                }
                onTextChanged: { /* no automatic persistence */ }
            }

            Label { text: qsTr("Amount"); Layout.preferredWidth: 80 }
            Controls.TextField {
                id: amountField; Layout.preferredWidth: 160
                onActiveFocusChanged: {
                    if (!activeFocus) persistCurrentTransaction()
                }
                onTextChanged: { /* no automatic persistence */ }
            }
        }

        RowLayout { Layout.fillWidth: true
            Label { text: qsTr("Type"); Layout.preferredWidth: 120 }
            Controls.TextField {
                id: typeField; Layout.fillWidth: true
                placeholderText: qsTr("Type")
                onActiveFocusChanged: { /* no editing marker handling */ }
                onTextChanged: { /* no automatic persistence */ }
            }
        }

        GroupBox {
            title: qsTr("Properties")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                ListView {
                    id: propertyListView
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: contentHeight
                    interactive: true
                    model: uiData ? uiData.properties : null

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter

                        Controls.CheckBox {
                            id: propCheck
                            Layout.preferredWidth: 28
                            Layout.margins: 2
                            checked: selectedPropertyIds.indexOf(model.id) !== -1
                            onClicked: {
                                if (!model.id) return
                                var localIdx = selectedPropertyIds.indexOf(model.id)
                                if (propCheck.checked) { if (localIdx === -1) selectedPropertyIds.push(model.id) }
                                else { if (localIdx > -1) selectedPropertyIds.splice(localIdx, 1) }

                                if (isEdit && current && current.id) {
                                    persistCurrentTransaction()
                                    if (uiData) uiData.setTransactionPropertyIdsImmediate(current.id, selectedPropertyIds)
                                } else {
                                }
                            }
                        }

                        ColumnLayout { Layout.fillWidth: true
                            Label { text: model.name }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Label {
                    id: propEmptyLabel
                    Layout.fillWidth: true
                    visible: propertyListView.count === 0
                    text: qsTr("No properties available")
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Controls.CheckBox {
                id: allocCheck
                text: qsTr("Allocatable to tenant")
                checked: false
                onClicked: persistCurrentTransaction()
            }

            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            Controls.ComboBox {
                id: statusCombo
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: 2
                onActivated: persistCurrentTransaction()
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true

            Controls.Button {
                text: qsTr("Back")
                onClicked: { if (uiData) uiData.selectedTransactionId = "" }
            }

            Controls.Button {
                visible: !isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0 && ((isEdit && current && current.statementId && current.statementId.length > 0) || (uiData && uiData.selectedStatementId && uiData.selectedStatementId.length > 0))
                onClicked: {
                    if (!transactionController) return
                    var amt = parseFloat(amountField.text)
                    if (isNaN(amt)) amt = 0.0

                    var sid = effectiveStatementId()
                    var id = transactionController.addTransaction(nameField.text,
                                                                  bookingDateField.text,
                                                                  amt,
                                                                  "",
                                                                  sid,
                                                                  statusCombo.currentIndex,
                                                                  "",
                                                                  allocCheck.checked,
                                                                  selectedPropertyIds)
                    if (id && id.length > 0) {
                        clearFields()
                        if (uiData) uiData.selectedTransactionId = id
                    }
                }
            }

            Controls.Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!transactionController) return
                    transactionController.deleteTransaction(current.id)
                    if (uiData) uiData.selectedTransactionId = ""
                    clearFields()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: syncFields()
}
