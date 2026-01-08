import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"

Item {
    id: root

    property bool hasUiImport: typeof uiImport !== 'undefined'
    property var draft: (hasUiImport && uiImport.draft) ? uiImport.draft : null

    function refreshActorUi() {
        if (actorCombo) actorCombo.syncFromDraft()
        if (actorNameField && draft && draft.current) {
            if (draft.current.actorId && draft.current.actorId.length > 0) {
                actorNameField.text = ""
            } else {
                actorNameField.text = draft.current.actorProposal || ""
            }
        }
    }

    function autoAssignAndRefreshActorUi() {
        if (typeof uiDomain !== 'undefined' && uiDomain && draft) {
            try {
                uiDomain.autoAssignActorsForDraft(draft)
            } catch (e) {
                // ignore
            }
        }
        // allow model notifications to propagate before syncing
        Qt.callLater(function() { refreshActorUi() })
    }

    function toFileUrl(p) {
        if (!p || p.length === 0) return ""
        if (p.indexOf("file://") === 0) return p
        var s = String(p)
        s = s.replace(/\\/g, "/")
        if (/^[A-Za-z]:\//.test(s)) return "file:///" + s
        if (s.indexOf("//") === 0) return "file:" + s
        return "file:///" + s
    }

    function currentProofUrl() {
        return toFileUrl(draft && draft.current ? (draft.current.proofImagePath || "") : "")
    }

    onDraftChanged: {
        if (draft) autoAssignAndRefreshActorUi()
    }

    Connections {
        target: draft
        function onChanged() {
            autoAssignAndRefreshActorUi()
        }
    }

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: pageLayout
            width: pageScroll.availableWidth
            anchors.margins: Theme.spacing
            spacing: Theme.spacing

            Label {
                text: qsTr("Import")
                font.pointSize: 18
            }

            GroupBox {
                visible: !draft
                title: qsTr("Import PDF")
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    ComboBox {
                        id: typeCombo
                        Layout.fillWidth: true
                        model: [ qsTr("Statement (PDF)") ]
                        currentIndex: 0
                        enabled: !(hasUiImport && uiImport.isRunning)
                    }

                    ComboBox {
                        id: profileCombo
                        Layout.fillWidth: true
                        model: hasUiImport ? uiImport.profiles : [ qsTr("Default") ]
                        enabled: !(hasUiImport && uiImport.isRunning)
                        Component.onCompleted: {
                            if (hasUiImport) currentIndex = Math.max(0, model.indexOf(uiImport.selectedProfile))
                        }
                        onActivated: {
                            if (hasUiImport) uiImport.selectedProfile = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        AppTextField {
                            id: fileField
                            Layout.fillWidth: true
                            placeholderText: qsTr("PDF file path")
                            enabled: !(hasUiImport && uiImport.isRunning)
                            text: hasUiImport ? (uiImport.selectedFile && uiImport.selectedFile.length > 0 ? uiImport.selectedFile : "P:/.data/fossredder/Januar 2025.pdf") : "P:/.data/fossredder/Januar 2025.pdf"
                            Component.onCompleted: {
                                if (hasUiImport && (!uiImport.selectedFile || uiImport.selectedFile.length === 0)) uiImport.selectedFile = "P:/.data/fossredder/Januar 2025.pdf"
                            }
                            onTextChanged: {
                                if (hasUiImport) uiImport.selectedFile = text
                            }
                        }

                        Button {
                            text: qsTr("Browse")
                            enabled: !(hasUiImport && uiImport.isRunning)
                            onClicked: {
                                if (uiActions) uiActions.browseImportPdf()
                            }
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        value: hasUiImport ? uiImport.progress : 0
                        visible: hasUiImport && (uiImport.isRunning || uiImport.progress > 0)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: hasUiImport ? (uiImport.error && uiImport.error.length > 0 ? uiImport.error : uiImport.phase) : ""
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        // When running, show Cancel button. Otherwise show Start + Reset
                        Button {
                            visible: hasUiImport && uiImport.isRunning
                            text: qsTr("Cancel")
                            enabled: hasUiImport && uiImport.isRunning
                            onClicked: {
                                if (hasUiImport) uiImport.cancelImport()
                            }
                        }

                        Button {
                            visible: hasUiImport && !uiImport.isRunning
                            text: qsTr("Start")
                            enabled: hasUiImport && !uiImport.isRunning && fileField.text.length > 0
                            onClicked: {
                                if (hasUiImport) uiImport.selectedFile = fileField.text
                                uiImport.startStatementImport()
                            }
                        }

                        Button {
                            visible: hasUiImport && !uiImport.isRunning
                            text: qsTr("Reset")
                            enabled: hasUiImport && !uiImport.isRunning
                            onClicked: uiImport.resetStatus()
                        }

                        // Show a small busy indicator while stopping is in progress
                        BusyIndicator {
                            running: hasUiImport && uiImport.phase === "Stopping..."
                            visible: running
                            width: 24
                            height: 24
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            GroupBox {
                visible: !!draft
                title: qsTr("Statement Draft")
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Name"); Layout.preferredWidth: 80 }
                        AppTextField { Layout.fillWidth: true; text: draft ? draft.name : ""; onTextChanged: if (draft) draft.name = text }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: draft ? (qsTr("Transaction %1 / %2").arg(draft.currentIndex + 1).arg(draft.count)) : ""
                    }

                    GroupBox {
                        title: qsTr("Transaction")
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

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
                                     id: proofWrap
                                     width: proofLoader.width
                                     implicitHeight: proofImage.status === Image.Ready ? proofImage.implicitHeight : 0

                                     Image {
                                         id: proofImage
                                         width: parent.width
                                         fillMode: Image.PreserveAspectFit
                                         source: currentProofUrl()
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
                                        id: metadataField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 160
                                        wrapMode: TextArea.Wrap
                                        text: draft && draft.current ? (draft.current.metadata || "") : ""
                                        selectByMouse: true
                                        onTextChanged: if (draft) draft.transactions.setMetadata(draft.currentIndex, text)
                                    }
                                }
                            }

                            GroupBox {
                                title: qsTr("Actor")
                                Layout.fillWidth: true

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6

                                    ComboBox {
                                        id: actorCombo
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
                                            id: actorNameField
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
                                            enabled: (typeof uiDomain !== 'undefined') && !!uiDomain && actorNameField.text.trim().length > 0
                                            onClicked: {
                                                if ((typeof uiDomain === 'undefined') || !uiDomain || !draft) return
                                                var id = uiDomain.ensureActorByName(actorNameField.text)
                                                if (id && id.length > 0) {
                                                    draft.transactions.setActorId(draft.currentIndex, id)
                                                    refreshActorUi()
                                                }
                                            }
                                        }

                                        Button {
                                            text: qsTr("Clear")
                                            enabled: draft && draft.current && ((draft.current.actorId && draft.current.actorId.length > 0) || (actorNameField.text.length > 0))
                                            onClicked: {
                                                if (!draft) return
                                                draft.transactions.setActorId(draft.currentIndex, "")
                                                draft.transactions.setActorProposal(draft.currentIndex, "")
                                                refreshActorUi()
                                            }
                                        }

                                        Button {
                                            text: qsTr("Auto")
                                            enabled: !!draft && !!uiDomain
                                            onClicked: {
                                                autoAssignAndRefreshActorUi()
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

                            // NEW: Allocatable checkbox and Status selector
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                CheckBox {
                                    id: allocCheck
                                    text: qsTr("Allocatable to tenant")
                                    checked: draft && draft.current ? !!draft.current.allocatable : false
                                    onCheckedChanged: if (draft) draft.transactions.setAllocatable(draft.currentIndex, checked)
                                }

                                Item { Layout.fillWidth: true }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
                                ComboBox {
                                    id: statusCombo
                                    Layout.fillWidth: true
                                    model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                                    currentIndex: draft && draft.current ? Math.max(0, draft.current.status) : 2
                                    onActivated: if (draft) draft.transactions.setStatus(draft.currentIndex, currentIndex)
                                }

                                Item { Layout.fillWidth: true }
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Button {
                                    text: qsTr("Cancel")
                                    onClicked: {
                                        if (hasUiImport) uiImport.clearDraft()
                                    }
                                }

                                Button {
                                    text: qsTr("Prev")
                                    enabled: draft && draft.currentIndex > 0
                                    onClicked: {
                                        if (draft) draft.prev()
                                        autoAssignAndRefreshActorUi()
                                        if (proofLoader) proofLoader.resetIfNeeded()
                                        Qt.callLater(function() {})
                                    }
                                }

                                Button {
                                    text: qsTr("Next")
                                    enabled: draft && (draft.currentIndex < draft.count - 1)
                                    onClicked: {
                                        if (draft) draft.next()
                                        autoAssignAndRefreshActorUi()
                                        if (proofLoader) proofLoader.resetIfNeeded()
                                        Qt.callLater(function() {})
                                    }
                                }

                                Button {
                                    text: qsTr("Finish")
                                    enabled: !!draft
                                    onClicked: {
                                        if (!draft) return
                                        if (typeof uiDomain !== 'undefined' && uiDomain) {
                                            var sid = uiDomain.finalizeStatementDraft(draft)
                                            if (hasUiImport) uiImport.clearDraft()
                                            if (sid && sid.length > 0 && uiNav && uiData) {
                                                uiNav.section = UiNavigation.Booking
                                                uiNav.bookingView = UiNavigation.Statements
                                                uiData.selectedStatementId = sid
                                                uiData.selectedTransactionId = ""
                                            }
                                            return
                                        }
                                        if (hasUiImport) uiImport.clearDraft()
                                    }
                                }

                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }

                Connections {
                    target: uiActions
                    function onImportFileSelected(path) {
                        if (hasUiImport) uiImport.selectedFile = path
                    }
                }
            }
        }
    }
}
