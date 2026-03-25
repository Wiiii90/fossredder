import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: stmtRoot
    anchors.fill: parent

    property var draft

    implicitHeight: stmtLayout.implicitHeight
    implicitWidth: stmtLayout.implicitWidth

    function discardDraft() {
        if (typeof importController !== 'undefined' && importController) importController.clearDraft()
        if (typeof uiData !== 'undefined' && uiData) {
            uiData.selectedStatementId = ""
            uiData.selectedTransactionId = ""
        }
        if (typeof uiNav !== 'undefined' && uiNav) uiNav.section = UiNavigation.Import
    }

    function finalizeDraft() {
        if (!draft || typeof draftController === 'undefined' || !draftController) return

        var sid = draftController.finalizeStatementDraft(draft)
        if (typeof importController !== 'undefined' && importController) importController.clearDraft()

        if (!(sid && sid.length > 0 && uiNav && uiData)) return

        uiData.selectedStatementId = sid
        try {
            var transactions = uiData.statementTransactionIds(sid)
            if (transactions && transactions.length > 0) uiData.selectedTransactionId = transactions[0]
        } catch(e) {
        }

        Qt.callLater(function() {
            uiNav.section = UiNavigation.Booking
            try { uiNav.bookingView = UiNavigation.Statements } catch(e) {}
        })
    }

    ColumnLayout {
        id: stmtLayout
        anchors.fill: parent
        spacing: Theme.spacingSmall

        Label {
            visible: !draft
            text: qsTr("No drafts available!")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        ColumnLayout {
            visible: draft
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Statement")
                    Layout.preferredWidth: Theme.chartValueLabelWidth
                }

                Controls.TextField {
                    Layout.fillWidth: true
                    text: draft ? draft.name : ""
                    onTextEdited: if (draft) draft.name = text
                }
            }

            Label {
                Layout.fillWidth: true
                text: (draft && draft.current)
                        ? (qsTr("Transaction %1 / %2").arg(draft.currentIndex + 1).arg(draft.count))
                        : qsTr("No current transaction")
            }

            Controls.Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.bottomMargin: Theme.spacingMedium
                contentSpacing: 0

                ScrollView {
                    id: txScroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 0
                    Layout.bottomMargin: Theme.spacingSmall
                    clip: true

                    TransactionDraftView {
                        id: txView
                        width: txScroll.availableWidth > 0 ? txScroll.availableWidth : txScroll.width
                        draft: stmtRoot.draft
                    }
                }
            }
        }        
    }
}
