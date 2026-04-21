import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var transactionController: root.appContext ? root.appContext.transactionController : null
    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false

    property bool isNew: false
    property var current: null
    property bool isEdit: false
    property var propertyRowsSnapshot: []
    property bool propertyRowsReady: false
    property bool propertiesExpanded: false

    function refreshCurrentSelection() {
        root.isNew = root.session && root.session.selectedTransactionId === "__new__"
        root.current = (!root.isNew && root.session) ? root.session.selectedTransaction : null
        root.isEdit = !root.isNew && root.current && root.current.id && String(root.current.id).length > 0
    }

    function loadPropertyRows() {
        root.propertyRowsSnapshot = root.session ? root.session.propertyRows() : []
        root.propertyRowsReady = true
    }

    function togglePropertiesExpanded() {
        root.propertiesExpanded = !root.propertiesExpanded
        if (root.propertiesExpanded && !root.propertyRowsReady)
            Qt.callLater(root.loadPropertyRows)
    }

    function hasSelectedProperty(propertyId) {
        return propertyId && root.selectedPropertyIds.indexOf(propertyId) !== -1
    }

    function toggleSelectedProperty(propertyId, checked) {
        if (!propertyId) return

        let nextPropertyIds = root.selectedPropertyIds.slice()
        const localIdx = nextPropertyIds.indexOf(propertyId)

        if (checked) {
            if (localIdx === -1) nextPropertyIds.push(propertyId)
        } else if (localIdx > -1) {
            nextPropertyIds.splice(localIdx, 1)
        }

        root.selectedPropertyIds = nextPropertyIds

        if (root.isEdit && root.current && root.current.id) {
            root.persistCurrentTransaction()
            if (root.session) root.session.setTransactionPropertyIdsImmediate(root.current.id, root.selectedPropertyIds)
        }
    }

    function clearFields() {
        nameField.text = ""
        bookingDateField.text = ""
        amountField.text = ""
        typeField.text = ""
        root.selectedPropertyIds = []
        allocCheck.checked = false
        statusCombo.currentIndex = 0
    }

    function syncFields() {
        
        if (root.isNew) {
            root.clearFields()
            return
        }
        if (!root.isEdit) {
            root.clearFields()
            return
        }
        

        nameField.text = root.current.name || ""
        bookingDateField.text = root.current.bookingDate || ""
        amountField.text = String(root.current.amount)
        typeField.text = root.current.type || ""
        root.selectedPropertyIds = root.current.propertyIds ? root.current.propertyIds.slice() : []
        allocCheck.checked = root.current.allocatable ? true : false
        statusCombo.currentIndex = Math.max(0, root.current.status)
    }

    function effectiveStatementId() {
        return (root.current && root.current.statementId && root.current.statementId.length > 0)
                ? root.current.statementId
                : ((root.session && root.session.selectedStatementId) ? root.session.selectedStatementId : "")
    }

    function persistCurrentTransaction() {
        if (root.isNew || !root.transactionController || !root.current || !root.current.id) return
        let amt = parseFloat(amountField.text)
        if (isNaN(amt)) amt = 0.0
        const sid = root.effectiveStatementId()
        const actorId = (root.current && root.current.actorId) ? root.current.actorId : ""
        const desc = (root.current && root.current.description) ? root.current.description : ""
        root.transactionController.updateTransaction(root.current.id,
                                               nameField.text,
                                               bookingDateField.text,
                                               amt,
                                               desc,
                                               sid,
                                               statusCombo.currentIndex,
                                               actorId,
                                               allocCheck.checked,
                                                root.selectedPropertyIds)
    }

    property var selectedPropertyIds: []

    Connections { target: root.current; function onChanged() { root.syncFields() } }

    Connections {
        target: root.session
        function onSelectedTransactionIdChanged() {
            Qt.callLater(function() {
                root.refreshCurrentSelection()
                root.syncFields()
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
                Layout.preferredHeight: root.propertyRowsReady ? propertyColumn.implicitHeight : 0
                interactive: true
                clip: true
                contentWidth: width
                contentHeight: root.propertyRowsReady ? propertyColumn.implicitHeight : 0

                Column {
                    id: propertyColumn
                    width: propertyListView.width
                    spacing: 2

                    Repeater {
                        model: root.propertyRowsReady ? root.propertyRowsSnapshot : []

                        delegate: Rectangle {
                            id: propertyRow
                            width: propertyColumn.width
                            required property var modelData
                            height: 32
                            radius: 6
                            color: root.hasSelectedProperty(propertyRow.modelData.id) ? root.theme.selectionHighlight : "transparent"
                            border.color: root.theme.borderSoft
                            border.width: root.theme.borderWidthThin

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.toggleSelectedProperty(propertyRow.modelData.id, !root.hasSelectedProperty(propertyRow.modelData.id))
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: root.theme.spacingSmall
                                spacing: root.theme.spacingSmall

                                Rectangle {
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16
                                    radius: 3
                                    color: root.hasSelectedProperty(propertyRow.modelData.id) ? root.theme.textPrimary : "transparent"
                                    border.color: root.theme.borderSoft
                                    border.width: root.theme.borderWidthThin
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: propertyRow.modelData.name ? propertyRow.modelData.name : ""
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: root.propertyRowsReady && root.propertyRowsSnapshot.length === 0
                text: qsTr("No properties available")
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Flickable {
            id: txScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: txContent.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: txContent
                width: txScroll.width
                spacing: 10

                Label { text: (root.isNew || root.isEdit) ? qsTr("Transaction") : qsTr("Create Transaction"); font.pointSize: 18 }

                RowLayout { Layout.fillWidth: true
                    Label { text: qsTr("Name"); Layout.preferredWidth: 120 }
                    Controls.TextField {
                        id: nameField; Layout.fillWidth: true
                        onActiveFocusChanged: {
                            if (!activeFocus) root.persistCurrentTransaction()
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
                            if (!activeFocus) root.persistCurrentTransaction()
                        }
                        onTextChanged: { /* no automatic persistence */ }
                    }

                    Label { text: qsTr("Amount"); Layout.preferredWidth: 80 }
                    Controls.TextField {
                        id: amountField; Layout.preferredWidth: 160
                        onActiveFocusChanged: {
                            if (!activeFocus) root.persistCurrentTransaction()
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
                    spacing: root.theme.spacingSmall

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Properties")
                            font.pointSize: root.theme.fontSizeTitle
                        }

                        Controls.Button {
                            text: root.propertiesExpanded ? qsTr("Hide") : qsTr("Show")
                            onClicked: root.togglePropertiesExpanded()
                        }
                    }

                    Loader {
                        Layout.fillWidth: true
                        active: root.propertiesExpanded
                        sourceComponent: propertySelectionComp
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Controls.CheckBox {
                        id: allocCheck
                        text: qsTr("Allocatable to tenant")
                        checked: false
                        onClicked: root.persistCurrentTransaction()
                    }

                    Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
                    Controls.ComboBox {
                        id: statusCombo
                        Layout.fillWidth: true
                        model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                        currentIndex: 2
                        onActivated: root.persistCurrentTransaction()
                    }
                    Item { Layout.fillWidth: true }
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                text: qsTr("Back")
                onClicked: { if (root.session) root.session.selectedTransactionId = "" }
            }

            Controls.Button {
                visible: !root.isEdit
                text: qsTr("Create")
                enabled: nameField.text.length > 0 && ((root.isEdit && root.current && root.current.statementId && root.current.statementId.length > 0) || (root.session && root.session.selectedStatementId && root.session.selectedStatementId.length > 0))
                onClicked: {
                    if (!root.transactionController) return
                    let amt = parseFloat(amountField.text)
                    if (isNaN(amt)) amt = 0.0

                    const sid = root.effectiveStatementId()
                    const id = root.transactionController.addTransaction(nameField.text,
                                                                  bookingDateField.text,
                                                                  amt,
                                                                  "",
                                                                  sid,
                                                                  statusCombo.currentIndex,
                                                                  "",
                                                                  allocCheck.checked,
                                                                  root.selectedPropertyIds)
                    if (id && id.length > 0) {
                        root.clearFields()
                        if (root.session) root.session.selectedTransactionId = id
                    }
                }
            }

            Controls.Button {
                visible: root.isEdit
                text: qsTr("Delete")
                onClicked: {
                    if (!root.transactionController) return
                    root.transactionController.deleteTransaction(root.current.id)
                    if (root.session) root.session.selectedTransactionId = ""
                    root.clearFields()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    Component.onCompleted: {
        Qt.callLater(function() {
            root.refreshCurrentSelection()
            root.syncFields()
        })
    }
}
