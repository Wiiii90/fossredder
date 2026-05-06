/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualTransactionsPanel.qml
 * @brief Provides the AnnualTransactionsPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property var transactions: []

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Annual transactions")
            Layout.fillWidth: true
        }

        Flickable {
            id: txScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: txRows.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: txRows
                width: txScroll.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.transactions

                    delegate: Rectangle {
                        id: txRow
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 34
                        radius: root.theme.radius
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.border

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.rightMargin: root.theme.spacingSmall
                            spacing: root.theme.spacingSmall

                            Label {
                                text: txRow.modelData && txRow.modelData.name ? txRow.modelData.name : ""
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: txRow.modelData && txRow.modelData.bookingDate ? txRow.modelData.bookingDate : ""
                                Layout.preferredWidth: 100
                                elide: Text.ElideRight
                            }

                            Label {
                                text: txRow.modelData && txRow.modelData.statusText ? txRow.modelData.statusText : ""
                                Layout.preferredWidth: 90
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Label {
                    visible: (root.transactions ? root.transactions.length : 0) === 0
                    text: qsTr("No annual transactions")
                    color: root.theme.textMuted
                    Layout.fillWidth: true
                }
            }
        }
    }
}
