/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualVerificationPanel.qml
 * @brief Provides the AnnualVerificationPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property int assignedAnalysisCount: 0
    property int transactionCount: 0
    property var issues: ({ missingFromYear: 0, mixedInAnnual: 0, duplicateCount: 0, missingLive: 0 })
    property var statusMetrics: ({ neutral: 0, unverified: 0, verified: 0, completed: 0 })

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Statistics")
            Layout.fillWidth: true
        }

        Controls.Panel {
            Layout.fillWidth: true
            contentSpacing: root.theme.spacingSmall

            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.theme.border
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Included transactions")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.transactionCount)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Duplicate entries")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.issues && root.issues.duplicateCount !== undefined ? root.issues.duplicateCount : 0)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Missing live transactions from selected year")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.issues && root.issues.missingFromYear !== undefined ? root.issues.missingFromYear : 0)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Included deleted transactions")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.issues && root.issues.missingLive !== undefined ? root.issues.missingLive : 0)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Transactions from a different year than selected")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: (root.issues && root.issues.mixedInAnnual !== undefined && root.issues.mixedInAnnual === 0)
                               ? root.theme.success
                               : root.theme.danger
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.issues && root.issues.mixedInAnnual !== undefined ? root.issues.mixedInAnnual : 0)
                        color: (root.issues && root.issues.mixedInAnnual !== undefined && root.issues.mixedInAnnual === 0)
                               ? root.theme.success
                               : root.theme.danger
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Assigned analyses")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.minimumWidth: root.theme.formLabelWidth / 2
                        horizontalAlignment: Text.AlignRight
                        text: String(root.assignedAnalysisCount)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall

                    Label {
                        text: qsTr("Statuses")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        text: qsTr("Neutral: %1, Unverified: %2, Verified: %3, Completed: %4")
                              .arg(root.statusMetrics && root.statusMetrics.neutral !== undefined
                                   ? root.statusMetrics.neutral : 0)
                              .arg(root.statusMetrics && root.statusMetrics.unverified !== undefined
                                   ? root.statusMetrics.unverified : 0)
                              .arg(root.statusMetrics && root.statusMetrics.verified !== undefined
                                   ? root.statusMetrics.verified : 0)
                              .arg(root.statusMetrics && root.statusMetrics.completed !== undefined
                                   ? root.statusMetrics.completed : 0)
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

    }
}
