import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"
import FossRedder 1.0

Item {
    id: stmtRoot

    property var draft
    implicitHeight: stmtLayout.implicitHeight
    implicitWidth: stmtLayout.implicitWidth

    ColumnLayout {
        id: stmtLayout
        width: parent.width

        Label {
            visible: !draft
            text: qsTr("No draft available")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        ColumnLayout {
            visible: draft

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Name")
                    Layout.preferredWidth: 80
                }

                AppTextField {
                    Layout.fillWidth: true
                    text: draft ? draft.name : ""
                    onTextChanged: if (draft) draft.name = text
                }
            }

            Label {
                Layout.fillWidth: true
                text: (draft && draft.current)
                        ? (qsTr("Transaction %1 / %2").arg(draft.currentIndex + 1).arg(draft.count))
                        : qsTr("No current transaction")
            }

            TransactionDraftView {
                id: txView
                Layout.fillWidth: true
                draft: stmtRoot.draft
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true

                AppButton {
                    text: qsTr("Prev")
                    enabled: draft && draft.currentIndex > 0
                    onClicked: {
                        if (draft) draft.prev()
                        txView.forceSync && txView.forceSync()
                    }
                }

                AppButton {
                    text: qsTr("Next")
                    enabled: draft && (draft.currentIndex < draft.count - 1)
                    onClicked: {
                        if (draft) draft.next()
                        txView.forceSync && txView.forceSync()
                    }
                }

                Item { Layout.fillWidth: true }

                AppButton {
                    text: qsTr("Discard")
                    enabled: !!draft
                    onClicked: {
                        // clear the import draft and return to Import view
                        if (typeof uiImport !== 'undefined' && uiImport) uiImport.clearDraft()
                        if (typeof uiData !== 'undefined' && uiData) {
                            uiData.selectedStatementId = ""
                            uiData.selectedTransactionId = ""
                        }
                        if (typeof uiNav !== 'undefined' && uiNav) uiNav.section = UiNavigation.Import
                    }
                }

                AppButton {
                    text: qsTr("Finish")
                    enabled: !!draft
                    onClicked: {
                        if (!draft) return;
                        if (typeof uiDomain !== 'undefined' && uiDomain) {
                            var sid = uiDomain.finalizeStatementDraft(draft)

                            // Clear the UI import draft so ImportView switches back
                            if (typeof uiImport !== 'undefined' && uiImport) uiImport.clearDraft()

                            if (sid && sid.length > 0 && uiNav && uiData) {
                                // stay on statements view and select the created statement
                                uiData.selectedStatementId = sid
                                uiData.selectedTransactionId = ""
                                Qt.callLater(function() {
                                    uiNav.section = UiNavigation.Booking
                                    uiNav.bookingView = UiNavigation.Statements
                                })
                            }
                            return
                        }
                    }
                }
            }
        }        
    }
}