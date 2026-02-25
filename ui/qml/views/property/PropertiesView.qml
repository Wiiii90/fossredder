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

        // rest of file omitted for brevity
