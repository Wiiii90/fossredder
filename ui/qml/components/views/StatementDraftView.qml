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
                    text: qsTr("Finish")
                    enabled: !!draft
                    onClicked: {
                        if (!draft) return;
                        if (typeof uiDomain !== 'undefined' && uiDomain) {
                            var sid = uiDomain.finalizeStatementDraft(draft)
                            if (sid && sid.length > 0 && uiNav && uiData) {
                                uiNav.section = UiNavigation.Booking
                                uiNav.bookingView = UiNavigation.Statements
                                uiData.selectedStatementId = sid
                                uiData.selectedTransactionId = ""
                            }
                            return
                        }
                    }
                }
            }
        }        
    }
}