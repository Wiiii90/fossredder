/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualTransactionsPanel.qml
 * @brief Provides the Annual transactions panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var annualState

    Layout.fillWidth: true
    Layout.fillHeight: true
    contentSpacing: root.theme.spacingSmall

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Annual transactions")
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radius
            color: root.theme.surfaceAlt
            border.width: root.theme.borderWidthThin
            border.color: root.theme.border

            Flickable {
                id: txScroll
                anchors.fill: parent
                anchors.margins: root.theme.spacingSmall
                clip: true
                contentWidth: txRows.width
                contentHeight: txRows.implicitHeight

                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                Column {
                    id: txRows
                    width: Math.max(txScroll.width, root.theme.annual.transactions.tableMinWidth)
                    spacing: root.theme.spacingSmall

                    Repeater {
                        model: root.annualState.transactionSections

                        delegate: Column {
                            id: sectionColumn
                            required property var modelData
                            width: txRows.width
                            spacing: root.theme.spacingSmall
                            visible: sectionColumn.modelData.visible

                            Rectangle {
                                objectName: "annualTransactionsSectionToggle_" + String(sectionColumn.modelData.key)
                                width: sectionColumn.width
                                height: root.theme.controlHeight
                                radius: root.theme.radius
                                color: root.theme.surface
                                border.width: root.theme.borderWidthThin
                                border.color: root.theme.border

                                Label {
                                    anchors.fill: parent
                                    anchors.leftMargin: root.theme.spacingSmall
                                    anchors.rightMargin: root.theme.spacingSmall
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    text: (sectionColumn.modelData.expanded ? "\u25BE " : "\u25B8 ")
                                          + sectionColumn.modelData.title + " (" + sectionColumn.modelData.rows.length + ")"
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                }

                                MouseArea {
                                    objectName: "annualTransactionsSectionMouseArea_" + String(sectionColumn.modelData.key)
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.annualState.toggleTransactionSection(sectionColumn.modelData.key)
                                }
                            }

                            Repeater {
                                model: sectionColumn.modelData.expanded ? sectionColumn.modelData.rows : []

                                delegate: Rectangle {
                                    id: txRow
                                    required property var modelData
                                    width: sectionColumn.width
                                    implicitHeight: rowLayout.implicitHeight + (root.theme.spacingSmall * 2)
                                    radius: root.theme.radius
                                    color: root.theme.surface
                                    border.width: root.theme.borderWidthThin
                                    border.color: txRow.modelData.isMixedYear
                                                  ? root.theme.danger
                                                  : root.theme.border

                                    RowLayout {
                                        id: rowLayout
                                        anchors.fill: parent
                                        anchors.margins: root.theme.spacingSmall
                                        spacing: root.theme.spacingSmall

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: root.theme.spacingSmall

                                            Label {
                                                text: txRow.modelData.name
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }

                                            Label {
                                                visible: txRow.modelData.sourceNamesText.length > 0
                                                text: qsTr("From: %1").arg(txRow.modelData.sourceNamesText)
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                                color: root.theme.textMuted
                                            }
                                        }

                                        Label {
                                            text: txRow.modelData.bookingDate
                                            Layout.preferredWidth: root.theme.annual.transactions.dateColumnWidth
                                            horizontalAlignment: Text.AlignRight
                                            elide: Text.ElideRight
                                        }

                                        Label {
                                            text: txRow.modelData.amountText
                                            Layout.preferredWidth: root.theme.annual.transactions.amountColumnWidth
                                            horizontalAlignment: Text.AlignRight
                                        }

                                        Label {
                                            text: txRow.modelData.allocatableText
                                            Layout.preferredWidth: root.theme.annual.transactions.allocatableColumnWidth
                                            horizontalAlignment: Text.AlignRight
                                            color: txRow.modelData.allocatable ? root.theme.success : root.theme.danger
                                            elide: Text.ElideRight
                                        }

                                        Label {
                                            text: txRow.modelData.contractTypeLabel
                                            Layout.preferredWidth: root.theme.annual.transactions.typeColumnWidth
                                            horizontalAlignment: Text.AlignRight
                                            elide: Text.ElideRight
                                            color: txRow.modelData.contractType.length > 0
                                                   ? root.theme.textPrimary
                                                   : root.theme.textMuted
                                        }

                                        Label {
                                            text: txRow.modelData.statusText
                                            Layout.preferredWidth: root.theme.annual.transactions.statusColumnWidth
                                            horizontalAlignment: Text.AlignRight
                                            color: txRow.modelData.statusTone === "success"
                                                   ? root.theme.success
                                                   : txRow.modelData.statusTone === "info"
                                                     ? root.theme.info
                                                     : txRow.modelData.statusTone === "warning"
                                                       ? root.theme.warning
                                                       : root.theme.textPrimary
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        visible: root.annualState.annualTransactions.length === 0
                        text: qsTr("No annual transactions")
                        color: root.theme.textMuted
                        width: txRows.width
                        leftPadding: root.theme.spacingSmall
                        rightPadding: root.theme.spacingSmall
                        topPadding: root.theme.spacingSmall
                        bottomPadding: root.theme.spacingSmall
                    }
                }
            }
        }
    }
}
