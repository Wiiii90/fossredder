import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    id: root
    // Allow this view to be sized by a parent layout
    Layout.fillWidth: true
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
        // get full sums from uiData, prefer transactionSumsForProperty, fallback to WithType
        if (uiData) {
            try { sums = uiData.transactionSumsForProperty(current.id) } catch(e) {
                try { sums = uiData.transactionSumsForPropertyWithType(current.id, "") } catch(e2) { sums = ({}) }
            }
        } else sums = ({})
        rebuildTypes()
        computeFilteredSums()
    }

    Connections { target: current; function onChanged() { syncFields() } }
    onIsEditChanged: syncFields()

    // listen for sums updates coming from the session
    property var sums: ({})
    Connections {
        target: uiData
        function onTransactionSumsUpdated(propertyId) {
            if (!current) return
            if (propertyId === current.id) {
                try { sums = uiData.transactionSumsForProperty(current.id) } catch(e) {
                    try { sums = uiData.transactionSumsForPropertyWithType(current.id, "") } catch(e2) { sums = ({}) }
                }
                rebuildTypes()
                computeFilteredSums()
            }
        }
    }

    // Filtering: selected transaction type (empty = all)
    property string txTypeFilter: ""
    // available types for current property's transactions
    property var txTypes: []
    // filtered sums when a filter is active
    property var sumsFiltered: ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })

    // shownSums reflects either the full sums or the filtered sums depending on txTypeFilter
    property var shownSums: ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })

    function updateShownSums() {
        shownSums = (txTypeFilter && txTypeFilter.length > 0) ? sumsFiltered : sums
    }

    onTxTypeFilterChanged: {
        // attempt to set txType on the proxy model; C++ invokable methods are callable directly
        try {
            var m = uiData ? uiData.transactionsForProperty(current.id) : null
            if (m) m.setTxType(txTypeFilter)
        } catch(e) { /* ignore if method not present */ }
        computeFilteredSums()
        updateShownSums()
    }

    onSumsChanged: updateShownSums()

    function rebuildTypes() {
        txTypes = []
        if (!current || !uiData) return
        // Prefer asking the session for unique type strings (safer across model types)
        try {
            var provided = uiData.transactionTypesForProperty(current.id)
            if (provided && provided.length !== undefined) { txTypes = provided; return }
        } catch(e) { /* ignore and fallback */ }

        // Fallback: iterate model if transactionTypesForProperty is not available
        var model = uiData.transactionsForProperty(current.id)
        if (!model) return
        var set = {}
        try {
            var cnt = (typeof model.count === 'function') ? model.count() : (model.length !== undefined ? model.length : 0)
            for (var i=0;i<cnt;i++) {
                var it = (typeof model.get === 'function') ? model.get(i) : (model[i] ? model[i] : null)
                if (!it) continue
                var t = it.type ? it.type : ""
                if (t && !set[t]) { set[t] = true; txTypes.push(t) }
            }
        } catch(e) { /* ignore */ }
    }

    function computeFilteredSums() {
        sumsFiltered = ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })
        if (!current || !uiData) { updateShownSums(); return }
        // if no filter selected, nothing to compute here
        if (!txTypeFilter || txTypeFilter.length === 0) { updateShownSums(); return }

        // Prefer asking the session for sums filtered by type (more reliable)
        if (typeof uiData.transactionSumsForPropertyWithType === 'function') {
            try {
                var res = uiData.transactionSumsForPropertyWithType(current.id, txTypeFilter)
                // ensure numeric values
                sumsFiltered.total = Number(res.total) || 0
                sumsFiltered.allocatable = Number(res.allocatable) || 0
                sumsFiltered.nonAllocatable = Number(res.nonAllocatable) || 0
                updateShownSums()
                return
            } catch(e) { /* fallback to model iterate below */ }
        }

        var model = uiData.transactionsForProperty(current.id)
        if (!model) { updateShownSums(); return }
        try {
            var cnt = (typeof model.count === 'function') ? model.count() : (model.length !== undefined ? model.length : 0)
            for (var i=0;i<cnt;i++) {
                var it = (typeof model.get === 'function') ? model.get(i) : (model[i] ? model[i] : null)
                if (!it) continue
                // model role 'type' may be provided as string
                var t = it.type ? it.type : ""
                if (t !== txTypeFilter) continue
                var amt = Number(it.amount) || 0
                sumsFiltered.total += amt
                if (it.allocatable) sumsFiltered.allocatable += amt; else sumsFiltered.nonAllocatable += amt
            }
        } catch(e) { /* ignore */ }
        updateShownSums()
    }

    ColumnLayout {
        id: mainLayout
        // let the layout fill the root item
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label { text: isEdit ? qsTr("Gebäudeübersicht") : qsTr("Neues Gebäude anlegen"); font.pointSize: 18 }

        AppTextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        Item { Layout.fillHeight: true }

        // Create-mode: only name and add button
        RowLayout {
            visible: !isEdit
            Layout.fillWidth: true



            AppButton {
                text: qsTr("Hinzufügen")
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

            // Filter row above the transactions list
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label { text: qsTr("Transaktionen"); Layout.alignment: Qt.AlignVCenter }

                Item { Layout.fillWidth: true }

                // filter button: shows available types in a menu (menu attached to button for correct positioning)
                AppButton {
                    id: filterButton
                    text: txTypeFilter && txTypeFilter.length > 0 ? txTypeFilter : qsTr("Filter")
                    implicitWidth: 120
                    onClicked: {
                        // open the menu sibling anchored to the button position
                        try { typeMenu.open() } catch(e) { /* ignore */ }
                    }
                }

                // Menu defined as a sibling to avoid assigning to non-existent 'menu' property
                Menu {
                    id: typeMenu
                    // rebuild types when menu opens
                    onOpened: { rebuildTypes(); }

                    MenuItem {
                        text: qsTr("Alle")
                        onTriggered: {
                            txTypeFilter = ""
                            computeFilteredSums()
                        }
                    }

                    Repeater {
                        model: txTypes
                        delegate: MenuItem {
                            text: modelData
                            onTriggered: {
                                txTypeFilter = modelData
                                computeFilteredSums()
                            }
                        }
                    }
                }
            }

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

                            // show booking date on the left and tx type on the right (safe when empty)
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Layout.alignment: Qt.AlignVCenter

                                Label {
                                    text: bookingDate || ""
                                    font.family: Theme.fontFamily
                                    font.pointSize: Theme.fontSize
                                    color: Theme.textMuted
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Label {
                                    // show type if present; keep muted style and safe for undefined/empty
                                    text: (type && type.length > 0) ? type : ""
                                    font.family: Theme.fontFamily
                                    font.pointSize: Math.max(12, Theme.fontSize - 2)
                                    color: Theme.textMuted
                                    horizontalAlignment: Text.AlignRight
                                    Layout.preferredWidth: 140
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }

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

                    // use the root-level shownSums (updated reactively)

                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter
                        width: amountColumnWidth
                        Label { text: qsTr("Umlegbar"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.allocatable !== undefined) ? (root.shownSums.allocatable).toFixed(2) : "0.00";
                            font.pointSize: 14; font.bold: true; color: "#4caf50"; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter
                        width: amountColumnWidth
                        Label { text: qsTr("Nicht-umlegbar"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.nonAllocatable !== undefined) ? (root.shownSums.nonAllocatable).toFixed(2) : "0.00";
                            font.pointSize: 14; font.bold: true; color: "#e53935"; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        width: amountColumnWidth
                        Label { text: qsTr("Summe"); font.pointSize: 12; color: Theme.textMuted; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.total !== undefined) ? (root.shownSums.total).toFixed(2) : "0.00";
                            font.pointSize: 16; font.bold: true; color: Theme.textPrimary; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true
                        }
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
                    AppButton { text: qsTr("Neues Gebäude anlegen"); onClicked: if (uiData) uiData.selectedPropertyId = "" }
                    AppButton { text: qsTr("Gebäude aktualisieren"); enabled: nameField.text.length > 0; onClicked: if (uiDomain) uiDomain.updateProperty(current.id, nameField.text, "", "") }
                }

                Item { Layout.fillWidth: true }

                // Right-side delete placed at the far right (bottom corner)
                AppButton {
                    text: qsTr("Löschen")
                    onClicked: if (uiDomain) { uiDomain.deleteProperty(current.id); if (uiData) uiData.selectedPropertyId = "" }
                    Layout.alignment: Qt.AlignRight
                    // red tint using the new per-instance properties (preserves hover/scale/glow behavior)
                    fillColor: "#e53935"
                    textColor: "white"
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
