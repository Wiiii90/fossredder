/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Import/StatementDraftView.qml
 * @brief Provides the StatementDraftView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: stmtRoot
    required property var appContext
    required property var theme

    readonly property var importController: stmtRoot.appContext ? stmtRoot.appContext.importController : null
    readonly property var draftController: stmtRoot.appContext ? stmtRoot.appContext.draftController : null
    readonly property var navigation: stmtRoot.appContext ? stmtRoot.appContext.navigation : null

    property var draft

    implicitHeight: stmtLayout.implicitHeight
    implicitWidth: stmtLayout.implicitWidth


    function returnToImport() {
        if (stmtRoot.importController && stmtRoot.importController.addRunNote)
            stmtRoot.importController.addRunNote(qsTr("Draft"), qsTr("Draft paused. Click log entry to continue."), true)
        if (stmtRoot.importController) stmtRoot.importController.clearDraft()
        if (stmtRoot.navigation) stmtRoot.navigation.setSectionValue(4)
    }

    function persistDraftSnapshotNow() {
        if (!stmtRoot.draft || !stmtRoot.draftController) return
        stmtRoot.draftController.persistStatementDraft(stmtRoot.draft)
    }

    function persistDraftSnapshot() {
        if (!stmtRoot.draft) return
        persistDebounce.restart()
    }

    function discardDraft() {
        const ic = stmtRoot.importController
        const dc = stmtRoot.draftController
        const nav = stmtRoot.navigation

        persistDebounce.stop()
        const draftId = (stmtRoot.draft && stmtRoot.draft.draftId) ? stmtRoot.draft.draftId : ""
        if (dc) dc.clearPersistedStatementDraft(draftId)
        if (ic && ic.addRunNote) ic.addRunNote(qsTr("Draft discarded"), qsTr("Statement draft was discarded."), false)
        if (ic) ic.clearDraft()
        if (nav) nav.setSectionValue(4)
    }

    function discardCurrentTransaction() {
        if (!stmtRoot.draft || stmtRoot.draft.count <= 1) return
        stmtRoot.draft.removeTransaction(stmtRoot.draft.currentIndex)
        stmtRoot.persistDraftSnapshot()
    }

    function deleteCurrentTransaction() {
        stmtRoot.discardCurrentTransaction()
    }

    function finalizeDraft() {
        if (!stmtRoot.draft || !stmtRoot.draftController) return

        const dc = stmtRoot.draftController
        const ic = stmtRoot.importController
        const nav = stmtRoot.navigation
        const d = stmtRoot.draft

        persistDebounce.stop()
        const sid = dc.finalizeStatementDraft(d)
        if (!(sid && sid.length > 0)) {
            if (ic && ic.addRunNote) ic.addRunNote(qsTr("Finalize failed"), qsTr("Draft could not be finalized."), true)
            return
        }

        dc.clearPersistedStatementDraft((d && d.draftId) ? d.draftId : "")
        if (ic && ic.addRunNote) ic.addRunNote(qsTr("Finalized"), qsTr("Draft was finalized into a statement."), false, sid)
        if (ic) ic.clearDraft()

        if (nav) {
            Qt.callLater(function() {
                nav.setSectionValue(4)
            })
        }
    }

    ColumnLayout {
        id: stmtLayout
        anchors.fill: parent
        spacing: stmtRoot.theme.spacingSmall

        Timer {
            id: persistDebounce
            interval: 350
            repeat: false
            onTriggered: stmtRoot.persistDraftSnapshotNow()
        }

        Component.onCompleted: {
            stmtRoot.persistDraftSnapshot()
        }

        Connections {
            target: stmtRoot.draft
            function onChanged() {
                stmtRoot.persistDraftSnapshot()
            }
        }

        Label {
            visible: !stmtRoot.draft
            text: qsTr("No drafts available!")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        ColumnLayout {
            visible: stmtRoot.draft
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: stmtRoot.theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Statement")
                    Layout.preferredWidth: stmtRoot.theme.chartValueLabelWidth
                }

                Controls.TextField {
                    Layout.fillWidth: true
                    text: stmtRoot.draft ? stmtRoot.draft.name : ""
                    onTextChanged: if (stmtRoot.draft && stmtRoot.draft.name !== text) stmtRoot.draft.name = text
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: (stmtRoot.draft && stmtRoot.draft.current)
                            ? (qsTr("Transaction %1 / %2").arg(stmtRoot.draft.currentIndex + 1).arg(stmtRoot.draft.count))
                            : qsTr("No current transaction")
                }

                Controls.SecondaryButton {
                    visible: stmtRoot.draft && stmtRoot.draft.count > 1
                    text: "×"
                    implicitHeight: stmtRoot.theme.viewCompactActionButtonSize
                    implicitWidth: stmtRoot.theme.viewCompactActionButtonSize
                    textColor: stmtRoot.theme.textMuted
                    onClicked: stmtRoot.deleteCurrentTransaction()
                }
            }

            Controls.Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentSpacing: 0

                ScrollView {
                    id: txScroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 0
                    clip: true

                    TransactionDraftView {
                        id: txView
                        width: txScroll.availableWidth > 0 ? txScroll.availableWidth : txScroll.width
                        appContext: stmtRoot.appContext
                        theme: stmtRoot.theme
                        draft: stmtRoot.draft
                    }
                }
            }
        }
    }
}
