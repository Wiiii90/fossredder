/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftAllocatablePanel.qml
 * @brief Controls allocatable settings and related draft suggestions for the current transaction.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: Theme.spacingSmall

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : Theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Theme.spacingSmall

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

            Rectangle {
                anchors.fill: parent
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
            }

            Text {
                anchors.centerIn: parent
                text: root.txRoot && root.txRoot.effectiveAllocatable() ? qsTr("Allocatable") : qsTr("Not allocatable")
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pointSize: Theme.fontSize
                font.bold: false
                font.weight: Font.Normal
            }

            MouseArea {
                anchors.fill: parent
                onClicked: if (root.txRoot && root.txRoot.draft) {
                    root.txRoot.draft.transactions.setAllocatable(root.txRoot.draft.currentIndex, !root.txRoot.effectiveAllocatable())
                    root.txRoot.draft.transactions.setAllocatableManualOverride(root.txRoot.draft.currentIndex, true)
                    if (root.txRoot.draft.refresh) root.txRoot.draft.refresh()
                }
            }
        }

        Label {
            text: root.txRoot && root.txRoot.contractTopSuggestion().label
                ? qsTr("Confidence: %1% — %2").arg(root.txRoot.suggestionConfidencePercent(root.txRoot.contractTopSuggestion())).arg(root.txRoot.contractTopSuggestion().label)
                : qsTr("No contract suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.contractTopSuggestion()) : Theme.textMuted
            Layout.fillWidth: true
        }
    }
}
