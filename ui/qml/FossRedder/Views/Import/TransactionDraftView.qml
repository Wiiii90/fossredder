import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Utils 1.0 as Utils

Item {
    id: txRoot

    property var draft
    function currentTransaction() {
        return draft && draft.current ? draft.current : null
    }

    function proofSource(p) {
        var s = p || ""
        if (s.length === 0) return ""
        if (s.indexOf("proof/") === 0) return "image://importProof/" + s
        return Utils.FileUtils.toFileUrl(s)
    }
    function formatAmount(a) {
        if (a === undefined || a === null) return "";
        try {
            var v = Number(a);
            if (!isFinite(v)) return "";
            var s = v.toFixed(2);
            s = s.replace('.', ',');
            return s;
        } catch (e) { return String(a); }
    }

    function updateAmount(text) {
        if (!draft) return
        var value = text ? text.trim() : ""
        if (value.length === 0) {
            draft.transactions.setAmount(draft.currentIndex, 0.0)
            return
        }
        try {
            if (value.indexOf(',') !== -1) {
                value = value.replace(/\./g, '')
            }
            value = value.replace(/,/g, '.')
            value = value.replace(/\s/g, '')
        } catch(e) {
        }
        var amount = parseFloat(value)
        if (isNaN(amount)) amount = 0.0
        draft.transactions.setAmount(draft.currentIndex, amount)
    }

    function setPropertySelected(propertyId, checked) {
        if (!draft || !currentTransaction() || !propertyId) return

        var selected = currentTransaction().propertyIds ? currentTransaction().propertyIds.slice(0) : []
        var index = selected.indexOf(propertyId)

        if (checked) {
            if (index === -1) selected.push(propertyId)
        } else if (index !== -1) {
            selected.splice(index, 1)
        }

        draft.transactions.setProperties(draft.currentIndex, selected)
        if (draft.refresh) draft.refresh()
    }

    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth

    ColumnLayout {
        id: txLayout
        width: txRoot.width
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Label { text: qsTr("Name"); Layout.preferredWidth: Theme.chartValueLabelWidth }

            Controls.TextField {
                Layout.fillWidth: true
                text: draft && draft.current ? (draft.current.name || "") : ""
                onTextChanged: if (draft) draft.transactions.setName(draft.currentIndex, text)
            }
        }

        Loader {
            id: proofLoader
            Layout.fillWidth: true
            Layout.preferredHeight: item ? item.implicitHeight : 0
            active: !!draft && !!draft.current && (draft.current.proofImagePath || "").length > 0

            property string _lastPath: ""

            function resetIfNeeded() {
                var p = (draft && draft.current) ? (draft.current.proofImagePath || "") : ""
                if (p === _lastPath) return
                _lastPath = p
                active = false
                Qt.callLater(function() { proofLoader.active = p.length > 0 })
            }

            sourceComponent: Item {
                width: proofLoader.width
                implicitHeight: proofImage.status === Image.Ready ? proofImage.implicitHeight : 0

                Image {
                    id: proofImage
                    width: parent.width
                    fillMode: Image.PreserveAspectFit
                    source: txRoot.proofSource((draft && draft.current) ? (draft.current.proofImagePath || "") : "")
                    cache: true
                    asynchronous: true
                }
            }

            Component.onCompleted: resetIfNeeded()
        }

        Label { text: qsTr("Buchungsdatum"); Layout.alignment: Qt.AlignVCenter }
        Controls.TextField {
            Layout.fillWidth: true
            text: draft && draft.current ? (draft.current.bookingDate || "") : ""
            onTextChanged: if (draft) draft.transactions.setBookingDate(draft.currentIndex, text)
        }

        Label { text: qsTr("Betrag"); Layout.alignment: Qt.AlignVCenter }
        Controls.TextField {
            Layout.fillWidth: true
            text: currentTransaction() ? formatAmount(currentTransaction().amount) : ""
            onTextChanged: updateAmount(text)
        }

        Label { text: qsTr("Transaction type"); Layout.alignment: Qt.AlignVCenter }
        Controls.TextField {
            Layout.fillWidth: true
            text: draft && draft.current ? (draft.current.type || "") : ""
            placeholderText: qsTr("")
            onTextChanged: if (draft) draft.transactions.setType(draft.currentIndex, text)
        }

        GroupBox {
            title: qsTr("Properties")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.margins
                spacing: Theme.margins

                ListView {
                    id: propertyListView
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: contentHeight
                    interactive: false
                    model: uiData ? uiData.properties : null

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.margins
                        Layout.alignment: Qt.AlignVCenter

                        Controls.CheckBox {
                            id: propCheck
                            Layout.preferredWidth: 28
                            Layout.margins: Theme.margins
                            checked: (draft && draft.current && draft.current.propertyIds && model.id) ? draft.current.propertyIds.indexOf(model.id) !== -1 : false
                            onClicked: setPropertySelected(model.id, propCheck.checked)
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: model.name }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Label {
                    id: propEmptyLabel
                    Layout.fillWidth: true
                    visible: propertyListView.count === 0
                    text: qsTr("No properties available, please create new properties first!")
                }
            }
            }

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: allocCheck_local
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Allocatable")
                checked: draft && draft.current ? !!draft.current.allocatable : false
                onCheckedChanged: if (draft) draft.transactions.setAllocatable(draft.currentIndex, checked)
            }
            Item { Layout.fillWidth: true }
        }
    }
    function forceSync() { }
}
