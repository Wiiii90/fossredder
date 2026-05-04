/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementPanel.qml
 * @brief Provides the BookingStatementPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    required property var theme
    property string statementName: ""
    property bool readOnly: false
    property string transactionInfoText: ""
    property bool transactionDeleteVisible: false
    property bool transactionDeleteEnabled: false
    property bool transactionAddVisible: false
    property bool transactionAddEnabled: true
    signal statementNameEdited(string text)
    signal transactionDeleteRequested()
    signal transactionAddRequested()
    default property alias content: contentLayout.data

    Layout.fillWidth: true
    Layout.fillHeight: true

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Statement")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Controls.TextField {
                objectName: "bookingStatementNameField"
                Layout.fillWidth: true
                readOnly: root.readOnly
                text: root.statementName
                onTextEdited: root.statementNameEdited(text)
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: root.transactionInfoText
                color: root.theme.textMuted
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.SecondaryButton {
                objectName: "bookingStatementAddTransactionButton"
                visible: root.transactionAddVisible
                enabled: root.transactionAddEnabled
                text: qsTr("Add")
                implicitHeight: root.theme.viewCompactActionButtonSize
                implicitWidth: root.theme.viewCompactActionButtonSize
                textColor: root.theme.textMuted
                focusPolicy: Qt.NoFocus
                onClicked: root.transactionAddRequested()
            }

            Controls.SecondaryButton {
                objectName: "bookingStatementRemoveTransactionButton"
                visible: root.transactionDeleteVisible
                enabled: root.transactionDeleteEnabled
                text: qsTr("Remove")
                implicitHeight: root.theme.viewCompactActionButtonSize
                implicitWidth: root.theme.viewCompactActionButtonSize
                textColor: root.theme.textMuted
                focusPolicy: Qt.NoFocus
                onClicked: root.transactionDeleteRequested()
            }
        }

        Controls.Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentSpacing: 0
            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surface
                border.width: 1
                border.color: root.theme.border
            }

            Flickable {
                id: transactionScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: contentLayout.implicitHeight

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                ColumnLayout {
                    id: contentLayout
                    width: transactionScroll.width
                    spacing: root.theme.spacingSmall
                }
            }
        }
    }
}
