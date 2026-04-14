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
    readonly property var session: stmtRoot.appContext ? stmtRoot.appContext.session : null
    readonly property var navigation: stmtRoot.appContext ? stmtRoot.appContext.navigation : null

    property var draft

    implicitHeight: stmtLayout.implicitHeight
    implicitWidth: stmtLayout.implicitWidth

    function discardDraft() {
        if (stmtRoot.importController) stmtRoot.importController.clearDraft()
        if (stmtRoot.session) {
            stmtRoot.session.selectedStatementId = ""
            stmtRoot.session.selectedTransactionId = ""
        }
        if (stmtRoot.navigation) stmtRoot.navigation.setSectionValue(4)
    }

    function finalizeDraft() {
        if (!stmtRoot.draft || !stmtRoot.draftController) return

        const sid = stmtRoot.draftController.finalizeStatementDraft(stmtRoot.draft)
        if (stmtRoot.importController) stmtRoot.importController.clearDraft()

        if (!(sid && sid.length > 0 && stmtRoot.navigation && stmtRoot.session)) return

        stmtRoot.session.selectedStatementId = sid
        try {
            const transactions = stmtRoot.session.statementTransactionIds(sid)
            if (transactions && transactions.length > 0) stmtRoot.session.selectedTransactionId = transactions[0]
        } catch(e) {
        }

        Qt.callLater(function() {
            stmtRoot.navigation.setSectionValue(3)
            try { stmtRoot.navigation.setBookingViewValue(0) } catch(e) {}
        })
    }

    function forceSync() {
        try {
            if (txView && txView.syncViewState) txView.syncViewState()
        } catch(e) {
        }
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
                    Layout.fillWidth: true
                    text: stmtRoot.draft ? stmtRoot.draft.name : ""
                    onTextEdited: if (stmtRoot.draft) stmtRoot.draft.name = text
                }
            }

            Label {
                Layout.fillWidth: true
                text: (stmtRoot.draft && stmtRoot.draft.current)
                        ? (qsTr("Transaction %1 / %2").arg(stmtRoot.draft.currentIndex + 1).arg(stmtRoot.draft.count))
                        : qsTr("No current transaction")
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
