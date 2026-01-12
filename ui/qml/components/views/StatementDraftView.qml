import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"
import FossRedder 1.0

Item {
    id: stmtRoot

    property var draft
    implicitWidth: stmtLayout.implicitWidth

    ColumnLayout {
        id: stmtLayout
        width: parent.width

        Label {
            visible: !draft
            text: qsTr("Keine Entwürfe verfügbar!")
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
                    text: qsTr("Bankauszug")
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
                        ? (qsTr("Transaktion %1 / %2").arg(draft.currentIndex + 1).arg(draft.count))
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
                    text: qsTr("Vorherige")
                    enabled: draft && draft.currentIndex > 0
                    onClicked: {
                        if (draft) draft.prev()
                        txView.forceSync && txView.forceSync()
                    }
                }

                AppButton {
                    text: qsTr("Nächste")
                    enabled: draft && (draft.currentIndex < draft.count - 1)
                    onClicked: {
                        if (draft) draft.next()
                        txView.forceSync && txView.forceSync()
                    }
                }

                Item { Layout.fillWidth: true }

                AppButton {
                    text: qsTr("Verwerfen")
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
                    text: qsTr("Abschließen")
                    enabled: !!draft
                    onClicked: {
                        if (!draft) return;
                        if (typeof uiDomain !== 'undefined' && uiDomain) {
                            var sid = uiDomain.finalizeStatementDraft(draft)

                            // Clear the UI import draft so ImportView switches back
                            if (typeof uiImport !== 'undefined' && uiImport) uiImport.clearDraft()

                            if (sid && sid.length > 0 && uiNav && uiData) {
                                // After finalize navigate to Properties view (instead of Booking/Statements)
                                uiData.selectedStatementId = ""
                                uiData.selectedTransactionId = ""
                                Qt.callLater(function() {
                                    uiNav.section = UiNavigation.Properties
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