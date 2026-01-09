import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"
import FossRedder 1.0

Item {
    id: txRoot
    property var draft

    anchors.fill: parent
    Layout.fillWidth: true
    Layout.fillHeight: true

    function toFileUrl(p) {
        if (!p || p.length === 0) return ""
        if (p.indexOf("file://") === 0) return p
        var s = String(p)
        s = s.replace(/\\/g, "/")
        if (/^[A-Za-z]:\//.test(s)) return "file:///" + s
        if (s.indexOf("//") === 0) return "file:" + s
        return "file:///" + s
    }

    ColumnLayout {
        id: txLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8
        Layout.fillWidth: true
        Layout.fillHeight: true

        // Transaction header fields
        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Name"); Layout.preferredWidth: 80 }
            AppTextField {
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
                    source: toFileUrl((draft && draft.current) ? (draft.current.proofImagePath || "") : "")
                    cache: true
                    asynchronous: true
                }
            }

            Component.onCompleted: resetIfNeeded()
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 6
            rowSpacing: 6
            columnSpacing: 8

            Label { text: qsTr("Booking date"); Layout.alignment: Qt.AlignVCenter }
            AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? (draft.current.bookingDate || "") : ""
                onTextChanged: if (draft) draft.transactions.setBookingDate(draft.currentIndex, text)
            }

            Label { text: qsTr("Valuta"); Layout.alignment: Qt.AlignVCenter }
            AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? (draft.current.valuta || "") : ""
                onTextChanged: if (draft) draft.transactions.setValuta(draft.currentIndex, text)
            }

            Label { text: qsTr("Amount"); Layout.alignment: Qt.AlignVCenter }
            AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? String(draft.current.amount) : ""
                onTextChanged: {
                    if (!draft) return
                    var v = parseFloat(text)
                    if (isNaN(v)) v = 0.0
                    draft.transactions.setAmount(draft.currentIndex, v)
                }
            }
        }

        GroupBox {
            title: qsTr("Metadata")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6
                TextArea {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    wrapMode: TextArea.Wrap
                    text: draft && draft.current ? (draft.current.metadata || "") : ""
                    selectByMouse: true
                    onTextChanged: if (draft) draft.transactions.setMetadata(draft.currentIndex, text)
                }
            }
        }

        // Actor selection block
        GroupBox {
            title: qsTr("Actor")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                ComboBox {
                    id: actorCombo_local
                    Layout.fillWidth: true
                    model: uiData ? uiData.actors : null
                    textRole: "name"
                    valueRole: "id"
                    currentIndex: -1

                    function syncFromDraft() {
                        if (!draft || !draft.current) { currentIndex = -1; return }
                        var aid = draft.current.actorId || ""
                        if (!aid || aid.length === 0) { currentIndex = -1; return }
                        for (var i = 0; i < count; ++i) {
                            if (valueAt(i) === aid) { currentIndex = i; return }
                        }
                        currentIndex = -1
                    }

                    onActivated: {
                        if (!draft) return
                        draft.transactions.setActorId(draft.currentIndex, currentValue)
                    }

                    Component.onCompleted: syncFromDraft()
                    onCountChanged: syncFromDraft()
                }

                RowLayout {
                    Layout.fillWidth: true
                    AppTextField {
                        id: actorNameField_local
                        Layout.fillWidth: true
                        placeholderText: qsTr("Proposed/new actor name")
                        text: (draft && draft.current && (!draft.current.actorId || draft.current.actorId.length === 0)) ? (draft.current.actorProposal || "") : ""
                        onTextChanged: {
                            if (!draft) return
                            draft.transactions.setActorProposal(draft.currentIndex, text)
                        }
                    }

                    Button {
                        text: qsTr("Add")
                        enabled: (typeof uiDomain !== 'undefined') && !!uiDomain && actorNameField_local.text.trim().length > 0
                        onClicked: {
                            if ((typeof uiDomain === 'undefined') || !uiDomain || !draft) return
                            var id = uiDomain.ensureActorByName(actorNameField_local.text)
                            if (id && id.length > 0) {
                                draft.transactions.setActorId(draft.currentIndex, id)
                                txRoot.forceSync && txRoot.forceSync()
                            }
                        }
                    }

                    Button {
                        text: qsTr("Clear")
                        enabled: draft && draft.current && ((draft.current.actorId && draft.current.actorId.length > 0) || (actorNameField_local.text.length > 0))
                        onClicked: {
                            if (!draft) return
                            draft.transactions.setActorId(draft.currentIndex, "")
                            draft.transactions.setActorProposal(draft.currentIndex, "")
                            txRoot.forceSync && txRoot.forceSync()
                        }
                    }

                    Button {
                        text: qsTr("Auto")
                        enabled: !!draft && !!uiDomain
                        onClicked: {
                            if (uiDomain) try { uiDomain.autoAssignActorsForDraft(draft) } catch (e) {}
                            Qt.callLater(function() { txRoot.forceSync && txRoot.forceSync() })
                        }
                    }
                }
            }
        }

        GroupBox {
            title: qsTr("Description")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6
                TextArea {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    wrapMode: TextArea.Wrap
                    placeholderText: qsTr("Description")
                    text: draft && draft.current ? (draft.current.description || "") : ""
                    onTextChanged: if (draft) draft.transactions.setDescription(draft.currentIndex, text)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                id: allocCheck_local
                text: qsTr("Allocatable to tenant")
                checked: draft && draft.current ? !!draft.current.allocatable : false
                onCheckedChanged: if (draft) draft.transactions.setAllocatable(draft.currentIndex, checked)
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            ComboBox {
                id: statusCombo_local
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: draft && draft.current ? Math.max(0, draft.current.status) : 2
                onActivated: if (draft) draft.transactions.setStatus(draft.currentIndex, currentIndex)
            }
            Item { Layout.fillWidth: true }
        }

        // navigation buttons left to parent
    }

    // helper callable by parent to force a sync of UI from draft
    function forceSync() { if (actorCombo_local) actorCombo_local.syncFromDraft() }
}
