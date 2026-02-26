import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import components.controls 1.0 as Controls
import "qrc:/qml/utils/FileUtils.js" as FileUtils
import FossRedder 1.0

Item {
    id: txRoot

    property var draft
    function proofSource(p) {
        var s = p || ""
        if (s.length === 0) return ""
        if (s.indexOf("proof/") === 0) return "image://importProof/" + s
        return FileUtils.toFileUrl(s)
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
    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth

    ColumnLayout {
        id: txLayout
        width: txRoot.width

        RowLayout {
            Layout.fillWidth: true

            Label { text: qsTr("Name"); Layout.preferredWidth: 80 }

            Controls.AppTextField {
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
        Controls.AppTextField {
            Layout.fillWidth: true
            text: draft && draft.current ? (draft.current.bookingDate || "") : ""
            onTextChanged: if (draft) draft.transactions.setBookingDate(draft.currentIndex, text)
        }

        Label { text: qsTr("Betrag"); Layout.alignment: Qt.AlignVCenter }
        Controls.AppTextField {
            Layout.fillWidth: true
            text: draft && draft.current ? formatAmount(draft.current.amount) : ""
            onTextChanged: {
                if (!draft) return
                var s = text ? text.trim() : ""
                if (s.length === 0) {
                    draft.transactions.setAmount(draft.currentIndex, 0.0)
                    return
                }
                try {
                    if (s.indexOf(',') !== -1) {
                        s = s.replace(/\./g, '')
                    }
                    s = s.replace(/,/g, '.')
                    s = s.replace(/\s/g, '')
                } catch(e) { }
                var v = parseFloat(s)
                if (isNaN(v)) v = 0.0
                draft.transactions.setAmount(draft.currentIndex, v)
            }
        }

        Label { text: qsTr("Transaktions-Typ"); Layout.alignment: Qt.AlignVCenter }
        Controls.AppTextField {
            Layout.fillWidth: true
            text: draft && draft.current ? (draft.current.type || "") : ""
            placeholderText: qsTr("")
            onTextChanged: if (draft) draft.transactions.setType(draft.currentIndex, text)
        }

        GroupBox {
            title: qsTr("Gebäude")
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
                    interactive: false
                    model: uiData ? uiData.properties : null

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter

                        Controls.AppCheckBox {
                            id: propCheck
                            Layout.preferredWidth: 28
                            Layout.margins: 2
                            checked: (draft && draft.current && draft.current.propertyIds && model.id) ? draft.current.propertyIds.indexOf(model.id) !== -1 : false
                            onClicked: {
                                if (!draft || !draft.current || !model.id) return
                                if (propCheck.checked) {
                                    draft.transactions.setProperties(draft.currentIndex, [model.id])
                                } else {
                                    draft.transactions.setProperties(draft.currentIndex, [])
                                }
                                if (draft && draft.refresh) draft.refresh()
                            }
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
                    text: qsTr("Keine Gebäude verfügbar, bitte neue Gebäude anlegen!")
                }
            }
            }

        RowLayout {
            Layout.fillWidth: true
            Controls.AppCheckBox {
                id: allocCheck_local
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Umlegbar")
                checked: draft && draft.current ? !!draft.current.allocatable : false
                onCheckedChanged: if (draft) draft.transactions.setAllocatable(draft.currentIndex, checked)
            }
            Item { Layout.fillWidth: true }
        }
    }
    function forceSync() { }
}
