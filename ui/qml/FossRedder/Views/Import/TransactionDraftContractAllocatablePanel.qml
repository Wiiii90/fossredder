/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftContractAllocatablePanel.qml
 * @brief Renders the allocatable toggle block embedded in the contract panel.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var transactionState
    required property var theme

    readonly property int suggestionTone: root.transactionState.suggestionTone(root.transactionState.allocatableSuggestionConfidence)
    readonly property color suggestionColor: root.suggestionTone === 2 ? root.theme.successStrong : (root.suggestionTone === 1 ? root.theme.warning : root.theme.danger)
    readonly property string resolvedFontFamily: String(root.theme.fontFamily || "Sans Serif")
    readonly property real resolvedFontSize: Number(root.theme.fontSize || 10)

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
                border.color: root.suggestionColor
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
                        text: root.transactionState.effectiveAllocatable ? qsTr("Allocatable") : qsTr("Not allocatable")
                        color: root.theme.textPrimary
                        font.family: root.resolvedFontFamily
                        font.pointSize: root.resolvedFontSize
                    }

                    MouseArea {
                        objectName: "transactionDraftAllocatableToggle"
                        anchors.fill: parent
                        onClicked: root.transactionState.toggleAllocatable()
                    }
                }
            }
        }

        Label {
            objectName: "transactionDraftAllocatableSuggestionLabel"
            text: root.transactionState.allocatableSuggestionText
            color: root.suggestionColor
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.theme.spacingSmall
        }
    }
}
