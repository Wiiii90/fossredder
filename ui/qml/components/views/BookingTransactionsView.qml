import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"

Item {
    id: root

    property bool isNew: uiData && uiData.selectedTransactionId === "__new__"
    property var current: (!isNew && uiData) ? uiData.selectedTransaction : null
    property bool isEdit: !isNew && current && current.id && String(current.id).length > 0

    function clearFields() {
        nameField.text = ""
        bookingDateField.text = ""
        amountField.text = ""
        // statementId is taken from uiData.selectedStatementId when creating new
        // reset property selection for new
        if (typeof selectedPropertyIds !== 'undefined') selectedPropertyIds = []
        allocCheck.checked = false
        statusCombo.currentIndex = 0
    }

    function syncFields() {
        if (suppressSync) return
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
        // populate property selection (for display only)
        if (typeof selectedPropertyIds !== 'undefined') selectedPropertyIds = current.propertyIds ? current.propertyIds.slice() : []
        allocCheck.checked = current.allocatable ? true : false
        statusCombo.currentIndex = Math.max(0, current.status)
    }

    // local storage for new-transaction property assignments
    property var selectedPropertyIds: []
    // prevent immediate model-driven sync from overwriting user clicks
    property bool suppressSync: false
    Timer {
        id: syncTimer
        interval: 150
        repeat: false
        onTriggered: { suppressSync = false; syncFields(); }
    }

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
            AppTextField { id: nameField; Layout.fillWidth: true }
        }

        RowLayout { Layout.fillWidth: true
            Label { text: qsTr("Booking date"); Layout.preferredWidth: 120 }
            AppTextField { id: bookingDateField; Layout.fillWidth: true }

            Label { text: qsTr("Amount"); Layout.preferredWidth: 80 }
            AppTextField { id: amountField; Layout.preferredWidth: 160 }
        }

        // Properties section
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

                        AppCheckBox {
                            id: propCheck
                            Layout.preferredWidth: 28
                            Layout.margins: 2
                            checked: (isEdit && current && current.propertyIds && model.id) ? current.propertyIds.indexOf(model.id) !== -1 : (selectedPropertyIds.indexOf(model.id) !== -1)
                            onClicked: {
                                if (!model.id) return
                                if (isEdit && current && current.id) {
                                    // update persistent transaction properties immediately (user click only)
                                    var ids = current.propertyIds ? current.propertyIds.slice() : []
                                    var idx = ids.indexOf(model.id)
                                    if (propCheck.checked) { if (idx === -1) ids.push(model.id) }
                                    else { if (idx > -1) ids.splice(idx, 1) }
                                    if (uiDomain) uiDomain.updateTransactionProperties(current.id, ids)
                                } else {
                                    // modify local selection for new transaction
                                    var localIdx = selectedPropertyIds.indexOf(model.id)
                                    if (propCheck.checked) { if (localIdx === -1) selectedPropertyIds.push(model.id) }
                                    else { if (localIdx > -1) selectedPropertyIds.splice(localIdx, 1) }
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

        // Allocatable and status
        RowLayout {
            Layout.fillWidth: true

            AppCheckBox {
                id: allocCheck
                text: qsTr("Allocatable to tenant")
                checked: false
                onClicked: {
                    // suppress immediate model sync to keep UI responsive and avoid override
                    suppressSync = true
                    syncTimer.restart()
                    if (!isNew && uiDomain && current && current.id) uiDomain.updateTransactionAllocatable(current.id, allocCheck.checked)
                }
            }

            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            AppComboBox {
                id: statusCombo
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: 2
                onActivated: {
                    if (!isNew && uiDomain && current && current.id) uiDomain.updateTransactionStatus(current.id, currentIndex)
                }
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true

            AppButton {
                text: qsTr("Back")
                onClicked: { if (uiData) uiData.selectedTransactionId = "" }
            }

            AppButton {
                text: (isEdit ? qsTr("Update") : qsTr("Add"))
                enabled: nameField.text.length > 0 && ((isEdit && current && current.statementId && current.statementId.length > 0) || (uiData && uiData.selectedStatementId && uiData.selectedStatementId.length > 0))
                onClicked: {
                    if (!uiDomain) return
                    var amt = parseFloat(amountField.text)
                    if (isNaN(amt)) amt = 0.0

                    var sid = (isEdit && current && current.statementId && current.statementId.length > 0) ? current.statementId : ((uiData && uiData.selectedStatementId) ? uiData.selectedStatementId : "")
                    if (isEdit) {
                        uiDomain.updateTransaction(current.id, nameField.text, bookingDateField.text, amt, "", sid)
                        uiDomain.updateTransactionAllocatable(current.id, allocCheck.checked)
                        uiDomain.updateTransactionStatus(current.id, statusCombo.currentIndex)
                        // properties were updated inline when checkboxes changed
                    } else {
                        var id = uiDomain.addTransaction(nameField.text, bookingDateField.text, amt, "", sid)
                        if (id && id.length > 0) {
                            // apply allocatable/status/properties
                            uiDomain.updateTransactionAllocatable(id, allocCheck.checked)
                            uiDomain.updateTransactionStatus(id, statusCombo.currentIndex)
                            if (selectedPropertyIds && selectedPropertyIds.length > 0) uiDomain.updateTransactionProperties(id, selectedPropertyIds)
                            clearFields()
                            if (uiData) uiData.selectedTransactionId = id
                        }
                    }
                }
            }

            AppButton {
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
