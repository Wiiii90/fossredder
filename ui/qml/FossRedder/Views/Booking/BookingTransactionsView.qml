import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    Accessible.ignored: typeof isDebugBuild !== 'undefined' && isDebugBuild

    property bool isNew: false
    property var current: null
    property bool isEdit: false
    property var propertyRowsSnapshot: []
    property bool propertyRowsReady: false
    property bool propertiesExpanded: false

    function refreshCurrentSelection() {
        root.isNew = session && session.selectedTransactionId === "__new__"
        root.current = (!root.isNew && session) ? session.selectedTransaction : null
        root.isEdit = !root.isNew && root.current && root.current.id && String(root.current.id).length > 0
    }

    function loadPropertyRows() {
        propertyRowsSnapshot = session ? session.propertyRows() : []
        propertyRowsReady = true
    }

    function togglePropertiesExpanded() {
        propertiesExpanded = !propertiesExpanded
        if (propertiesExpanded && !propertyRowsReady)
            Qt.callLater(loadPropertyRows)
    }

    function hasSelectedProperty(propertyId) {
        return propertyId && selectedPropertyIds.indexOf(propertyId) !== -1
    }

    function toggleSelectedProperty(propertyId, checked) {
        if (!propertyId) return

        var nextPropertyIds = selectedPropertyIds.slice()
        var localIdx = nextPropertyIds.indexOf(propertyId)

        if (checked) {
            if (localIdx === -1) nextPropertyIds.push(propertyId)
        } else if (localIdx > -1) {
            nextPropertyIds.splice(localIdx, 1)
        }

        selectedPropertyIds = nextPropertyIds

        if (isEdit && current && current.id) {
            persistCurrentTransaction()
            if (session) session.setTransactionPropertyIdsImmediate(current.id, selectedPropertyIds)
        }
    }

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
                : ((session && session.selectedStatementId) ? session.selectedStatementId : "")
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

    Connections {
        target: session
        function onSelectedTransactionIdChanged() {
            Qt.callLater(function() {
                refreshCurrentSelection()
                syncFields()
            })
        }
    }

    Component {
        id: propertySelectionComp

        ColumnLayout {
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            spacing: 2

            Flickable {
                id: propertyListView
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: propertyRowsReady ? propertyColumn.implicitHeight : 0
                interactive: true
                clip: true
                contentWidth: width
                contentHeight: propertyRowsReady ? propertyColumn.implicitHeight : 0

                Column {
                    id: propertyColumn
                    width: propertyListView.width
                    spacing: 2

                    Repeater {
                        model: propertyRowsReady ? propertyRowsSnapshot : []

                        delegate: Rectangle {
                            width: propertyColumn.width
                            height: 32
                            radius: 6
                            color: hasSelectedProperty(modelData.id) ? Theme.selectionHighlight : "transparent"
                            border.color: Theme.borderSoft
                            border.width: Theme.borderWidthThin

                            MouseArea {
                                anchors.fill: parent
                                onClicked: toggleSelectedProperty(modelData.id, !hasSelectedProperty(modelData.id))
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: Theme.spacingSmall
                                spacing: Theme.spacingSmall

                                Rectangle {
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16
                                    radius: 3
                                    color: hasSelectedProperty(modelData.id) ? Theme.textPrimary : "transparent"
                                    border.color: Theme.borderSoft
                                    border.width: Theme.borderWidthThin
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.name ? modelData.name : ""
                                    color: Theme.textPrimary
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: propertyRowsReady && propertyRowsSnapshot.length === 0
                text: qsTr("No properties available")
            }
        }
    }

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

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Properties")
                    font.pointSize: Theme.fontSizeTitle
                }

                Controls.Button {
                    text: propertiesExpanded ? qsTr("Hide") : qsTr("Show")
                    onClicked: togglePropertiesExpanded()
                }
            }

            Loader {
                Layout.fillWidth: true
                active: propertiesExpanded
                sourceComponent: propertySelectionComp
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
                onClicked: { if (session) session.selectedTransactionId = "" }
            }

            Controls.Button {
                visible: !isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0 && ((isEdit && current && current.statementId && current.statementId.length > 0) || (session && session.selectedStatementId && session.selectedStatementId.length > 0))
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
                        if (session) session.selectedTransactionId = id
                    }
                }
            }

            Controls.Button {
                visible: isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!transactionController) return
                    transactionController.deleteTransaction(current.id)
                    if (session) session.selectedTransactionId = ""
                    clearFields()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        Qt.callLater(function() {
            refreshCurrentSelection()
            syncFields()
        })
    }
}
