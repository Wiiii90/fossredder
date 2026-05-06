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

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Assigned analyses")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.assignedAnalysisCount)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Snapshot transactions")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.transactionCount)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Missing year tx")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.issues && root.issues.missingFromYear !== undefined ? root.issues.missingFromYear : 0)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Mixed year tx")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.issues && root.issues.mixedInAnnual !== undefined ? root.issues.mixedInAnnual : 0)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Duplicates")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.issues && root.issues.duplicateCount !== undefined ? root.issues.duplicateCount : 0)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Missing live tx")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: String(root.issues && root.issues.missingLive !== undefined ? root.issues.missingLive : 0)
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Status")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Label {
                text: qsTr("N:%1 U:%2 V:%3 C:%4")
                      .arg(root.statusMetrics && root.statusMetrics.neutral !== undefined ? root.statusMetrics.neutral : 0)
                      .arg(root.statusMetrics && root.statusMetrics.unverified !== undefined ? root.statusMetrics.unverified : 0)
                      .arg(root.statusMetrics && root.statusMetrics.verified !== undefined ? root.statusMetrics.verified : 0)
                      .arg(root.statusMetrics && root.statusMetrics.completed !== undefined ? root.statusMetrics.completed : 0)
                Layout.fillWidth: true
            }
        }

    }
}
