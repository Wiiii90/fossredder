/**
 * @file ui/qml/FossRedder/Views/Import/StatementDraftView.qml
 * @brief Handles draft review, finalize, discard, and navigation actions for imported statements.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: stmtRoot
    required property var appContext
    required property var theme

    readonly property var importWorkflow: stmtRoot.appContext ? stmtRoot.appContext.importWorkflow : null
    readonly property var navigation: stmtRoot.appContext ? stmtRoot.appContext.navigation : null

    property var draft

    implicitHeight: stmtLayout.implicitHeight
    implicitWidth: stmtLayout.implicitWidth


    function returnToImport() {
        stmtRoot.persistPendingEdits()
        const draftId = (stmtRoot.draft && stmtRoot.draft.draftId) ? stmtRoot.draft.draftId : ""
        if (stmtRoot.importWorkflow && stmtRoot.importWorkflow.addRunNote)
            stmtRoot.importWorkflow.addRunNote(qsTr("Draft"), qsTr("Draft paused. Click log entry to continue."), true, "", draftId)
        if (stmtRoot.importWorkflow) stmtRoot.importWorkflow.clearDraft()
        if (stmtRoot.navigation) stmtRoot.navigation.setSectionValue(4)
    }

    function discardDraft() {
        const ic = stmtRoot.importWorkflow
        const dc = stmtRoot.importWorkflow
        const nav = stmtRoot.navigation

        const draftId = (stmtRoot.draft && stmtRoot.draft.draftId) ? stmtRoot.draft.draftId : ""
        if (dc) dc.clearPersistedStatementDraft(draftId)
        if (ic && ic.addRunNote) ic.addRunNote(qsTr("Draft discarded"), qsTr("Statement draft was discarded."), false, "", draftId)
        if (ic) ic.clearDraft()
        if (nav) nav.setSectionValue(4)
    }

    function discardCurrentTransaction() {
        if (!stmtRoot.draft || stmtRoot.draft.count <= 1) return
        stmtRoot.draft.removeTransaction(stmtRoot.draft.currentIndex)
        if (stmtRoot.importWorkflow)
            stmtRoot.importWorkflow.persistStatementDraft(stmtRoot.draft)
    }

    function deleteCurrentTransaction() {
        stmtRoot.discardCurrentTransaction()
    }

    function addTransactionAfterCurrent() {
        if (!stmtRoot.draft)
            return
        if (stmtRoot.draft.insertTransactionAfterCurrent)
            stmtRoot.draft.insertTransactionAfterCurrent()
        if (stmtRoot.importWorkflow)
            stmtRoot.importWorkflow.persistStatementDraft(stmtRoot.draft)
    }

    function finalizeDraft() {
        if (!stmtRoot.draft || !stmtRoot.importWorkflow) return
        stmtRoot.persistPendingEdits()

        const dc = stmtRoot.importWorkflow
        const ic = stmtRoot.importWorkflow
        const nav = stmtRoot.navigation
        const d = stmtRoot.draft
        const draftId = (d && d.draftId) ? d.draftId : ""

        const sid = dc.finalizeStatementDraft(d)
        if (!(sid && sid.length > 0)) {
            if (ic && ic.addRunNote) ic.addRunNote(qsTr("Finalize failed"), qsTr("Draft could not be finalized."), true, "", draftId)
            return
        }

        dc.clearPersistedStatementDraft(draftId)
        if (ic && ic.addRunNote) ic.addRunNote(qsTr("Finalized"), qsTr("Draft was finalized into a statement."), false, sid, draftId)
        if (ic) ic.clearDraft()

        if (nav) {
            Qt.callLater(function() {
                nav.setSectionValue(4)
            })
        }
    }

    function commitStatementName(value) {
        if (!stmtRoot.draft) return
        const nextValue = value !== undefined && value !== null ? String(value) : ""
        const currentValue = stmtRoot.draft.name !== undefined && stmtRoot.draft.name !== null ? String(stmtRoot.draft.name) : ""
        if (nextValue !== currentValue)
            stmtRoot.draft.name = nextValue
    }

    function commitPendingEdits() {
        stmtRoot.commitStatementName(statementDraftNameField.text)
        if (txView && txView.commitPendingEdits)
            txView.commitPendingEdits()
    }

    function persistPendingEdits() {
        stmtRoot.commitPendingEdits()
        if (stmtRoot.importWorkflow && stmtRoot.draft && stmtRoot.importWorkflow.persistStatementDraft)
            stmtRoot.importWorkflow.persistStatementDraft(stmtRoot.draft)
    }

    ColumnLayout {
        id: stmtLayout
        anchors.fill: parent
        spacing: stmtRoot.theme.spacingSmall

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
                    id: statementDraftNameField
                    objectName: "statementDraftNameField"
                    Layout.fillWidth: true
                    text: stmtRoot.draft ? stmtRoot.draft.name : ""
                    onEditingFinished: stmtRoot.commitStatementName(text)
                    onAccepted: stmtRoot.commitStatementName(text)
                    onActiveFocusChanged: if (!activeFocus) stmtRoot.commitStatementName(text)
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
                    objectName: "statementDraftAddTransactionButton"
                    visible: stmtRoot.draft
                    text: qsTr("+")
                    implicitHeight: stmtRoot.theme.viewCompactActionButtonSize
                    implicitWidth: stmtRoot.theme.viewCompactActionButtonSize
                    textColor: stmtRoot.theme.textMuted
                    focusPolicy: Qt.NoFocus
                    onClicked: stmtRoot.addTransactionAfterCurrent()
                }

                Controls.SecondaryButton {
                    objectName: "statementDraftDeleteTransactionButton"
                    visible: stmtRoot.draft && stmtRoot.draft.count > 1
                    text: qsTr("-")
                    implicitHeight: stmtRoot.theme.viewCompactActionButtonSize
                    implicitWidth: stmtRoot.theme.viewCompactActionButtonSize
                    textColor: stmtRoot.theme.textMuted
                    focusPolicy: Qt.NoFocus
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
