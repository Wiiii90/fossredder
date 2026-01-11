import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    id: root
    // Allow this view to be sized by a parent layout
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent

    // width for amount column so amounts and total align
    property int amountColumnWidth: 120

    // Toggle visual debug overlays when diagnosing layout issues
    property bool debugLayout: false

    property var current: uiData ? uiData.selectedProperty : null
    property bool isEdit: current && current.id && String(current.id).length > 0

    function clearFields() { nameField.text = ""; sums = ({}) }

    function syncFields() {
        if (!isEdit) { clearFields(); return }
        nameField.text = current.name || ""
        sums = uiData ? uiData.transactionSumsForProperty(current.id) : ({})
    }

    Connections { target: current; function onChanged() { syncFields() } }
    onIsEditChanged: syncFields()

    // listen for sums updates coming from the session
    property var sums: uiData ? (current && current.id ? uiData.transactionSumsForProperty(current.id) : ({})) : ({})
    Connections {
        target: uiData
        function onTransactionSumsUpdated(propertyId) {
            if (!current) return
            if (propertyId === current.id) sums = uiData.transactionSumsForProperty(current.id)
        }
    }

    ColumnLayout {
        id: mainLayout
        // let the layout fill the root item
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label { text: isEdit ? qsTr("Edit Property") : qsTr("Create Property"); font.pointSize: 18 }

        AppTextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        // Create-mode: only name and add button
        RowLayout {
            visible: !isEdit
            Layout.fillWidth: true

            AppButton {
                text: qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: {
                    if (!uiDomain) return
                    var id = uiDomain.addProperty(nameField.text, "", "")
                    clearFields()
                    if (uiData && id && id.length > 0) uiData.selectedPropertyId = id
                }
            }

            Item { Layout.fillWidth: true }
        }

        // Edit-mode: name editable plus delete/new and transaction balance view
        ColumnLayout {
            visible: isEdit
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // Transactions list expands between nameField and buttons/sums
            ListView {
                id: txList
                clip: true
                interactive: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: uiData ? uiData.transactionsForProperty(current.id) : null
                spacing: 6

                delegate: Rectangle {
                    width: parent ? parent.width : txList.width
                    height: 56
                    color: Theme.surface
                    border.color: "#e6e6e6"
                    border.width: 1
                    radius: Theme.radius

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12
                        Layout.fillWidth: true

                        // status indicator
                        Rectangle {
                            id: statusIndicator
                            width: 12; height: 12; radius: 6
                            Layout.alignment: Qt.AlignVCenter
                            border.width: 1
                            color: (typeof status !== 'undefined') ? (status === 0 ? "#9e9e9e" : (status === 1 ? "#ff9800" : (status === 2 ? "#2196f3" : (status === 3 ? "#4caf50" : "#9e9e9e")))) : "#9e9e9e"
                            border.color: Qt.darker(color, 1.2)
                        }

                        // booking info column
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Layout.alignment: Qt.AlignVCenter

                            Label { text: bookingDate || ""; font.family: Theme.fontFamily; font.pointSize: Theme.fontSize; color: Theme.textMuted; Layout.alignment: Qt.AlignVCenter; Layout.fillWidth: true }
                            Label { text: description ? description : ""; font.family: Theme.fontFamily; font.pointSize: Math.max(12, Theme.fontSize - 2); color: Theme.textMuted; elide: Text.ElideRight; Layout.alignment: Qt.AlignVCenter; Layout.fillWidth: true }
                        }

                        // spacer to push amount right
                        Item { Layout.fillWidth: true }

                        // amount column
                        Label {
                            id: amtText
                            text: (amount || 0).toFixed(2)
                            font.family: Theme.fontFamily
                            font.pointSize: Theme.fontSize
                            font.bold: true
                            color: Theme.textPrimary
                            Layout.preferredWidth: amountColumnWidth
                            Layout.alignment: Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            // Sums: total / allocatable / non-allocatable
            Rectangle {
                color: Theme.surface
                border.color: "#e6e6e6"
                border.width: 1
                radius: Theme.radius
                Layout.fillWidth: true
                Layout.preferredHeight: 72

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 16
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    // Allocatable (fixed column)
                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter
                        width: amountColumnWidth
                        Label { text: qsTr("Allocatable"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label { text: sums && sums.allocatable !== undefined ? (sums.allocatable).toFixed(2) : "0.00"; font.pointSize: 14; font.bold: true; color: "#4caf50"; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                    }

                    // Non-allocatable (fixed column to right of Allocatable)
                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter
                        width: amountColumnWidth
                        Label { text: qsTr("Non-Allocatable"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label { text: sums && sums.nonAllocatable !== undefined ? (sums.nonAllocatable).toFixed(2) : "0.00"; font.pointSize: 14; font.bold: true; color: "#e53935"; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                    }

                    Item { Layout.fillWidth: true }

                    // Total aligned under amounts column (fixed)
                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        width: amountColumnWidth
                        Label { text: qsTr("Total"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                        Label { text: sums && sums.total !== undefined ? (sums.total).toFixed(2) : "0.00"; font.pointSize: 16; font.bold: true; color: Theme.textPrimary; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                    }
                }
            }

            // Buttons moved below the sums panel; New and Update on the left, Delete on the right
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                anchors.margins: 2
                spacing: 8

                // Left-side actions
                RowLayout {
                    spacing: 8
                    AppButton { text: qsTr("New"); onClicked: if (uiData) uiData.selectedPropertyId = "" }
                    AppButton { text: qsTr("Update"); enabled: nameField.text.length > 0; onClicked: if (uiDomain) uiDomain.updateProperty(current.id, nameField.text, "", "") }
                }

                Item { Layout.fillWidth: true }

                // Right-side delete placed at the far right (bottom corner)
                AppButton {
                    text: qsTr("Delete")
                    onClicked: if (uiDomain) { uiDomain.deleteProperty(current.id); if (uiData) uiData.selectedPropertyId = "" }
                    Layout.alignment: Qt.AlignRight
                    // red background and white label for emphasis
                    background: Rectangle { color: "#e53935"; radius: 4 }
                    contentItem: Label { text: qsTr("Delete"); color: "white"; horizontalAlignment: Text.AlignHCenter }
                }
            }

        }
    }

    // Debug label to help diagnose overlay issues (toggle debugLayout to true to show)
    Label {
        id: dbgCount
        visible: false
        text: qsTr("txCount: ") + (uiData && current ? (uiData.transactionsForProperty(current.id) ? (uiData.transactionsForProperty(current.id).count ? uiData.transactionsForProperty(current.id).count() : "?") : "-") : "-")
        anchors.top: parent.top
        anchors.right: parent.right
        color: "red"
    }
}
