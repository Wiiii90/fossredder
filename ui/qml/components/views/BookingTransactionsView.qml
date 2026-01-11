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
        // if user is actively editing this transaction, avoid overwriting fields
        if (uiData && current && current.id && uiData.isEditingTransaction(current.id)) return

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

    // debounce timer used to batch text edits before calling domain update
    Timer {
        id: updateTimer
        interval: 300
        repeat: false
        onTriggered: {
            if (!isNew && uiDomain && current && current.id) {
                var amt = parseFloat(amountField.text)
                if (isNaN(amt)) amt = 0.0
                var sid = (current && current.statementId && current.statementId.length > 0) ? current.statementId : ((uiData && uiData.selectedStatementId) ? uiData.selectedStatementId : "")
                uiDomain.updateTransaction(current.id, nameField.text, bookingDateField.text, amt, "", sid)
            }
        }
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
            AppTextField {
                id: nameField; Layout.fillWidth: true
                onActiveFocusChanged: {
                    if (!isNew && uiData) uiData.setEditingTransaction(current.id, activeFocus)
                }
                onTextChanged: {
                    // schedule inline update for existing transaction
                    if (!isNew && uiDomain && current && current.id) updateTimer.restart()
                }
            }
        }

        RowLayout { Layout.fillWidth: true
            Label { text: qsTr("Booking date"); Layout.preferredWidth: 120 }
            AppTextField {
                id: bookingDateField; Layout.fillWidth: true
                onActiveFocusChanged: { if (!isNew && uiData) uiData.setEditingTransaction(current.id, activeFocus) }
                onTextChanged: { if (!isNew && uiDomain && current && current.id) updateTimer.restart() }
            }

            Label { text: qsTr("Amount"); Layout.preferredWidth: 80 }
            AppTextField {
                id: amountField; Layout.preferredWidth: 160
                onActiveFocusChanged: { if (!isNew && uiData) uiData.setEditingTransaction(current.id, activeFocus) }
                onTextChanged: { if (!isNew && uiDomain && current && current.id) updateTimer.restart() }
            }
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
                            // use the local selectedPropertyIds array so repeated clicks accumulate
                            checked: selectedPropertyIds.indexOf(model.id) !== -1
                            onClicked: {
                                if (!model.id) return
                                // update local selection first
                                var localIdx = selectedPropertyIds.indexOf(model.id)
                                if (propCheck.checked) { if (localIdx === -1) selectedPropertyIds.push(model.id) }
                                else { if (localIdx > -1) selectedPropertyIds.splice(localIdx, 1) }

                                if (isEdit && current && current.id) {
                                    // apply persistent update immediately using accumulated local selection
                                    if (uiDomain) uiDomain.updateTransactionProperties(current.id, selectedPropertyIds)
                                    // also update UI model immediately so other views (PropertiesView) refresh without waiting for commit
                                    if (uiData) uiData.setTransactionPropertyIdsImmediate(current.id, selectedPropertyIds)
                                } else {
                                    // for new transaction we already modify local selection
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
                    // suppress immediate model sync briefly to avoid UI flicker
                    suppressSync = true
                    syncTimer.restart()
                    // mark editing to suppress incremental overwrite while the user interacts
                    if (!isNew && uiData) uiData.setEditingTransaction(current.id, true)
                    // update the domain model immediately so syncFields does not revert the toggle
                    if (!isNew && uiDomain && current && current.id) uiDomain.updateTransactionAllocatable(current.id, allocCheck.checked)
                    // clear editing marker shortly after to allow incremental updates again
                    if (!isNew) Qt.callLater(function() { Qt.createQmlObject('import QtQuick 2.0; Timer { interval: 400; repeat: false; running: true; onTriggered: { if (uiData) uiData.setEditingTransaction(current.id, false) } }', root)
                    })
                }
            }

            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            AppComboBox {
                id: statusCombo
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: 2
                onActivated: {
                    // mark editing to suppress incremental overwrite while the user interacts
                    if (!isNew && uiData) uiData.setEditingTransaction(current.id, true)
                    // apply status immediately to keep behavior consistent with other controls
                    if (!isNew && uiDomain && current && current.id) uiDomain.updateTransactionStatus(current.id, currentIndex)
                    // clear editing marker shortly after
                    if (!isNew) Qt.callLater(function() { Qt.createQmlObject('import QtQuick 2.0; Timer { interval: 400; repeat: false; running: true; onTriggered: { if (uiData) uiData.setEditingTransaction(current.id, false) } }', root)
                    })
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

            // For inline edits we hide the Update action and apply changes directly from fields.
            AppButton {
                visible: !isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0 && ((isEdit && current && current.statementId && current.statementId.length > 0) || (uiData && uiData.selectedStatementId && uiData.selectedStatementId.length > 0))
                onClicked: {
                    if (!uiDomain) return
                    var amt = parseFloat(amountField.text)
                    if (isNaN(amt)) amt = 0.0

                    var sid = (isEdit && current && current.statementId && current.statementId.length > 0) ? current.statementId : ((uiData && uiData.selectedStatementId) ? uiData.selectedStatementId : "")
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
