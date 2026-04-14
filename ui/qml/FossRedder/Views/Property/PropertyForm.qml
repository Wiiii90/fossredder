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
    readonly property var propertyController: root.appContext ? root.appContext.propertyController : null

    readonly property var current: root.session ? root.session.selectedProperty : null
    readonly property int amountColumnWidth: root.theme.formLabelWidth
    property bool isEdit: root.current && root.current.id && String(root.current.id).length > 0
    property var aliases: []

    function clearFields() { nameField.text = ""; aliasesField.text = ""; root.sums = ({}) }

    function refreshSums() {
        if (!root.current || !root.session) {
            root.sums = ({})
            return
        }
        try { root.sums = root.session.propertyTransactionSums(root.current.id) } catch(e) {
            try { root.sums = root.session.propertyTransactionSums(root.current.id, "") } catch(e2) { root.sums = ({}) }
        }
    }

    function submitProperty() {
        if (!root.propertyController) return
        const aliasValues = []
        if (aliasesField.text && aliasesField.text.length > 0) {
            const raw = aliasesField.text.split(/\r?\n/)
            for (let i = 0; i < raw.length; ++i) {
                const value = String(raw[i]).trim()
                if (value.length > 0 && aliasValues.indexOf(value) === -1) aliasValues.push(value)
            }
        }
        if (root.isEdit) {
            root.propertyController.updateProperty(root.current.id, nameField.text, "", "", aliasValues)
            return
        }
        const propertyId = root.propertyController.addProperty(nameField.text, "", "", aliasValues)
        root.clearFields()
        if (root.session && propertyId && propertyId.length > 0) root.session.selectedPropertyId = propertyId
    }

    function syncFields() {
        if (!root.isEdit) { root.clearFields(); return }
        nameField.text = root.current.name || ""
        aliasesField.text = (root.current.aliases || []).join("\n")
        root.refreshSums()
        root.rebuildTypes()
        root.computeFilteredSums()
    }

    Connections { target: root.current; function onChanged() { root.syncFields() } }
    onIsEditChanged: root.syncFields()

    property var sums: ({})
    Connections {
        target: root.session
        function onTransactionSumsUpdated(propertyId) {
            if (!root.current) return
            if (propertyId === root.current.id) {
                root.refreshSums()
                root.rebuildTypes()
                root.computeFilteredSums()
            }
        }
    }

    property string txTypeFilter: ""
    property var txTypes: []
    property var sumsFiltered: ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })
    property var shownSums: ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })

    function updateShownSums() { root.shownSums = (root.txTypeFilter && root.txTypeFilter.length > 0) ? root.sumsFiltered : root.sums }

    onTxTypeFilterChanged: {
        try { const m = root.session ? root.session.propertyTransactions(root.current.id) : null; if (m) m.setTxType(root.txTypeFilter) } catch(e) {}
        root.computeFilteredSums()
        root.updateShownSums()
    }

    onSumsChanged: root.updateShownSums()

    function rebuildTypes() {
        root.txTypes = []
        if (!root.current || !root.session) return
        try {
            const provided = root.session.propertyContractTypes(root.current.id)
            if (provided && provided.length !== undefined) { root.txTypes = provided; return }
        } catch(e) {}
        const model = root.session.propertyTransactions(root.current.id)
        if (!model) return
        const set = {}
        try {
            const cnt = (typeof model.count === 'function') ? model.count() : (model.length !== undefined ? model.length : 0)
            for (let i=0;i<cnt;i++) {
                const it = (typeof model.get === 'function') ? model.get(i) : (model[i] ? model[i] : null)
                if (!it) continue
                const t = it.type ? it.type : ""
                if (t && !set[t]) { set[t] = true; root.txTypes.push(t) }
            }
        } catch(e) {}
    }

    function computeFilteredSums() {
        root.sumsFiltered = ({ total:0.0, allocatable:0.0, nonAllocatable:0.0 })
        if (!root.current || !root.session) { root.updateShownSums(); return }
        if (!root.txTypeFilter || root.txTypeFilter.length === 0) { root.updateShownSums(); return }
        if (typeof root.session.propertyTransactionSums === 'function') {
            try {
                const res = root.session.propertyTransactionSums(root.current.id, root.txTypeFilter)
                root.sumsFiltered.total = Number(res.total) || 0
                root.sumsFiltered.allocatable = Number(res.allocatable) || 0
                root.sumsFiltered.nonAllocatable = Number(res.nonAllocatable) || 0
                root.updateShownSums()
                return
            } catch(e) {}
        }
        const model = root.session.propertyTransactions(root.current.id)
        if (!model) { root.updateShownSums(); return }
        try {
            const cnt = (typeof model.count === 'function') ? model.count() : (model.length !== undefined ? model.length : 0)
            for (let i=0;i<cnt;i++) {
                const it = (typeof model.get === 'function') ? model.get(i) : (model[i] ? model[i] : null)
                if (!it) continue
                const t = it.type ? it.type : ""
                if (t !== root.txTypeFilter) continue
                const amt = Number(it.amount) || 0
                root.sumsFiltered.total += amt
                if (it.allocatable) root.sumsFiltered.allocatable += amt; else root.sumsFiltered.nonAllocatable += amt
            }
        } catch(e) {}
        root.updateShownSums()
    }

    ColumnLayout {
        id: mainLayout
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        anchors.margins: root.theme.spacing
        spacing: root.theme.spacingMedium

        Label { text: root.isEdit ? qsTr("Building overview") : qsTr("Create new building"); font.pointSize: root.theme.fontSizeTitle + root.theme.margins }

        Controls.TextField { id: nameField; placeholderText: qsTr("Name"); Layout.fillWidth: true }

        GroupBox {
            title: qsTr("Aliases")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.theme.spacingMedium
                spacing: root.theme.spacingSmall

                Label {
                    text: qsTr("One alias per line. Used for auto-matching during import.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Controls.TextArea {
                    id: aliasesField
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.chartLegendHeight
                    wrapMode: TextArea.Wrap
                    placeholderText: qsTr("e.g.\nMain Building\nHQ\nSite A")
                }
            }
        }

        Item { Layout.fillHeight: true }

        ColumnLayout {
            visible: root.isEdit
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: root.theme.spacingMedium

            RowLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingMedium

                Label { text: qsTr("Transactions"); Layout.alignment: Qt.AlignVCenter }

                Item { Layout.fillWidth: true }

                Controls.Button {
                    id: filterButton
                    text: root.txTypeFilter && root.txTypeFilter.length > 0 ? root.txTypeFilter : qsTr("Filter")
                    implicitWidth: root.amountColumnWidth
                    onClicked: { try { typeMenu.open() } catch(e) {} }
                }

                Menu {
                    id: typeMenu
                    onOpened: { root.rebuildTypes(); }

                    MenuItem { text: qsTr("All"); onTriggered: { root.txTypeFilter = ""; root.computeFilteredSums() } }
                    Repeater { model: root.txTypes; delegate: MenuItem { required property var modelData; text: modelData; onTriggered: { root.txTypeFilter = modelData; root.computeFilteredSums() } } }
                }
            }

            ListView {
                id: txList
                clip: true
                interactive: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: root.session ? root.session.propertyTransactions(root.current.id) : null
                spacing: root.theme.spacingSmall

                delegate: Rectangle { id: txRow
                    required property real amount
                    required property string bookingDate
                    required property string description
                    required property int status
                    required property string type
                    width: parent ? parent.width : txList.width
                    height: 56
                    color: root.theme.surface
                    border.color: root.theme.borderSoft
                    border.width: root.theme.borderWidthThin
                    radius: root.theme.radius

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: root.theme.spacingMedium
                        spacing: root.theme.spacing
                        Layout.fillWidth: true

                        Rectangle {
                            id: statusIndicator
                            width: root.theme.spacing
                            height: root.theme.spacing
                            radius: root.theme.spacingSmall
                            Layout.alignment: Qt.AlignVCenter
                            border.width: root.theme.borderWidthThin
                            color: txRow.status === 0 ? root.theme.neutral
                                : (txRow.status === 1 ? root.theme.warning
                                   : (txRow.status === 2 ? root.theme.info
                                      : (txRow.status === 3 ? root.theme.successStrong : root.theme.neutral)))
                            border.color: Qt.darker(color, 1.2)
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: root.theme.margins
                            Layout.alignment: Qt.AlignVCenter

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: root.theme.spacingMedium
                                Layout.alignment: Qt.AlignVCenter

                                Label {
                                    text: txRow.bookingDate || ""
                                    font.family: root.theme.fontFamily
                                    font.pointSize: root.theme.fontSize
                                    color: root.theme.textMuted
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Label {
                                    text: (txRow.type && txRow.type.length > 0) ? txRow.type : ""
                                    font.family: root.theme.fontFamily
                                    font.pointSize: Math.max(12, root.theme.fontSize - 2)
                                    color: root.theme.textMuted
                                    horizontalAlignment: Text.AlignRight
                                    Layout.preferredWidth: 140
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }

                            Label { text: txRow.description ? txRow.description : ""; font.family: root.theme.fontFamily; font.pointSize: Math.max(12, root.theme.fontSize - 2); color: root.theme.textMuted; elide: Text.ElideRight; Layout.alignment: Qt.AlignVCenter; Layout.fillWidth: true }
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            id: amtText
                            text: (txRow.amount || 0).toFixed(2)
                            font.family: root.theme.fontFamily
                            font.pointSize: root.theme.fontSize
                            font.bold: true
                            color: root.theme.textPrimary
                            Layout.preferredWidth: root.amountColumnWidth
                            Layout.alignment: Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            Rectangle {
                color: root.theme.surface
                border.color: root.theme.borderSoft
                border.width: root.theme.borderWidthThin
                radius: root.theme.radius
                Layout.fillWidth: true
                Layout.preferredHeight: 72

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: root.theme.spacingMedium
                    spacing: root.theme.spacingLarge
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    ColumnLayout {
                        spacing: root.theme.margins
                        Layout.alignment: Qt.AlignVCenter
                        width: root.amountColumnWidth
                        Label { text: qsTr("Allocatable"); font.pointSize: 12; color: root.theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.allocatable !== undefined) ? (root.shownSums.allocatable).toFixed(2) : "0.00";
                            font.pointSize: 14; font.bold: true; color: root.theme.successStrong; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        spacing: root.theme.margins
                        Layout.alignment: Qt.AlignVCenter
                        width: root.amountColumnWidth
                        Label { text: qsTr("Non-allocatable"); font.pointSize: 12; color: root.theme.textMuted; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.nonAllocatable !== undefined) ? (root.shownSums.nonAllocatable).toFixed(2) : "0.00";
                            font.pointSize: 14; font.bold: true; color: root.theme.dangerStrong; horizontalAlignment: Text.AlignLeft; Layout.fillWidth: true
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout {
                        spacing: root.theme.margins
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        width: root.amountColumnWidth
                        Label { text: qsTr("Total"); font.pointSize: 12; color: root.theme.textMuted; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                        Label {
                            text: (root.shownSums && root.shownSums.total !== undefined) ? (root.shownSums.total).toFixed(2) : "0.00";
                            font.pointSize: 16; font.bold: true; color: root.theme.textPrimary; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        Components.BottomBar {
            Layout.fillWidth: true
            theme: root.theme

            Controls.Button {
                visible: !root.isEdit
                text: qsTr("Add")
                enabled: nameField.text.length > 0
                onClicked: root.submitProperty()
            }

            Controls.Button {
                visible: root.isEdit
                text: qsTr("Create new building")
                onClicked: if (root.session) root.session.selectedPropertyId = ""
            }

            Controls.Button {
                visible: root.isEdit
                text: qsTr("Update building")
                enabled: nameField.text.length > 0
                onClicked: root.submitProperty()
            }

            Controls.Button {
                visible: root.isEdit
                text: qsTr("Delete")
                onClicked: if (root.propertyController) { root.propertyController.deleteProperty(root.current.id); if (root.session) root.session.selectedPropertyId = "" }
                fillColor: root.theme.dangerStrong
            }

            Item { Layout.fillWidth: true }
        }
    }
}
