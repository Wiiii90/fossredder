/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractAllocatablePanel.qml
 * @brief Renders the allocatable toggle block embedded in the transaction draft contract panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    property var txRoot
    required property var theme
    readonly property string resolvedFontFamily: (root.theme && root.theme.fontFamily !== undefined && root.theme.fontFamily !== null && String(root.theme.fontFamily).length > 0) ? String(root.theme.fontFamily) : "Sans Serif"
    readonly property real resolvedFontSize: (root.theme && root.theme.fontSize !== undefined && root.theme.fontSize !== null) ? Number(root.theme.fontSize) : 10

    function toggleAllocatable() {
        if (!(root.txRoot && root.txRoot.draft))
            return
        root.txRoot.draft.transactions.setAllocatable(root.txRoot.draft.currentIndex, !root.txRoot.effectiveAllocatable())
        root.txRoot.draft.transactions.setAllocatableSelected(root.txRoot.draft.currentIndex, true)
        if (root.txRoot.draft.refresh) root.txRoot.draft.refresh()
    }

    function suggestionText() {
        if (!(root.txRoot && root.txRoot.viewState))
            return qsTr("No suggestion")
        const summary = root.txRoot.viewState.allocatableSuggestionSummary
        return summary && String(summary).length > 0 ? String(summary) : qsTr("0% Confidence - No suggestion")
    }

    function suggestionColor() {
        if (!(root.txRoot && root.txRoot.viewState))
            return root.theme.textMuted
        const confidence = Number(root.txRoot.viewState.allocatableSuggestionConfidence || 0)
        return root.txRoot.suggestionColor({ confidence: confidence })
    }

    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Controls.Panel {
            Layout.fillWidth: true
            background: Rectangle {
                radius: root.theme.radius
                color: root.theme.surfaceAlt
                border.width: 1
                border.color: root.suggestionColor()
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall

                Label {
                    text: qsTr("Select Allocatable")
                    Layout.fillWidth: true
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.theme.viewCompactActionButtonSize

                    Rectangle {
                        anchors.fill: parent
                        radius: root.theme.radius
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.border
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.txRoot && root.txRoot.effectiveAllocatable() ? qsTr("Allocatable") : qsTr("Not allocatable")
                        color: root.theme.textPrimary
                        font.family: root.resolvedFontFamily
                        font.pointSize: root.resolvedFontSize
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.toggleAllocatable()
                    }
                }
            }
        }

        Label {
            text: root.suggestionText()
            color: root.suggestionColor()
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
