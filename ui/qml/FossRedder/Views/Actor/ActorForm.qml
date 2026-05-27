/**
 * @file ui/qml/FossRedder/Views/Actor/ActorForm.qml
 * @brief Provides the ActorForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

pragma ComponentBehavior: Bound

Item {
    id: root
    required property var actorState
    required property var theme
    implicitHeight: formContent.implicitHeight

    ColumnLayout {
        id: formContent
        anchors.fill: parent
        spacing: root.theme.viewFormSpacing

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Actor Name")
                Layout.preferredWidth: root.theme.formLabelWidth
            }

            Controls.TextField {
                id: nameField
                objectName: "actorNameField"
                placeholderText: ""
                text: root.actorState ? root.actorState.name : ""
                Layout.fillWidth: true
                onTextEdited: if (root.actorState) root.actorState.name = text
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: root.theme.viewAliasGroupSpacing

            RowLayout {
                id: aliasControlsRow
                Layout.fillWidth: true
                readonly property real aliasControlSize: root.theme.viewCompactActionButtonSize

                Label {
                    text: qsTr("Aliases")
                    Layout.preferredWidth: root.theme.formLabelWidth
                }

                Controls.TextField {
                    id: actorAliasInput
                    objectName: "actorAliasInput"
                    Layout.fillWidth: true
                    placeholderText: ""
                    text: root.actorState ? root.actorState.aliasInputText : ""
                    onTextEdited: if (root.actorState) root.actorState.aliasInputText = text
                }

                Controls.SecondaryButton {
                    objectName: "actorAddAliasButton"
                    text: qsTr("+")
                    Layout.preferredWidth: aliasControlsRow.aliasControlSize
                    Layout.minimumWidth: aliasControlsRow.aliasControlSize
                    Layout.maximumWidth: aliasControlsRow.aliasControlSize
                    Layout.preferredHeight: aliasControlsRow.aliasControlSize
                    Layout.minimumHeight: aliasControlsRow.aliasControlSize
                    Layout.maximumHeight: aliasControlsRow.aliasControlSize
                    textColor: root.theme.textMuted
                    enabled: root.actorState ? root.actorState.canAddAlias(actorAliasInput.text) : false
                    onClicked: if (root.actorState) root.actorState.addAlias(actorAliasInput.text)
                }

                Controls.SecondaryButton {
                    objectName: "actorRemoveAliasButton"
                    text: qsTr("-")
                    Layout.preferredWidth: aliasControlsRow.aliasControlSize
                    Layout.minimumWidth: aliasControlsRow.aliasControlSize
                    Layout.maximumWidth: aliasControlsRow.aliasControlSize
                    Layout.preferredHeight: aliasControlsRow.aliasControlSize
                    Layout.minimumHeight: aliasControlsRow.aliasControlSize
                    Layout.maximumHeight: aliasControlsRow.aliasControlSize
                    textColor: root.theme.textMuted
                    enabled: root.actorState ? root.actorState.canRemoveSelectedAlias() : false
                    onClicked: if (root.actorState) root.actorState.requestRemoveSelectedAlias()
                }
            }

            Controls.Panel {
                Layout.fillWidth: true
                Layout.minimumHeight: root.theme.viewAliasPanelMinHeight
                Layout.preferredHeight: root.theme.viewAliasPanelPreferredHeight
                Layout.maximumHeight: root.theme.viewAliasPanelPreferredHeight
                contentSpacing: 0
                background: Rectangle {
                    radius: root.theme.radius
                    color: root.theme.surface
                    border.width: 1
                    border.color: root.theme.border
                }

                Flickable {
                    id: actorAliasScroll
                    objectName: "actorAliasScroll"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentWidth: width
                    contentHeight: actorAliasFlow.implicitHeight

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    Flow {
                        id: actorAliasFlow
                        width: actorAliasScroll.width
                        spacing: root.theme.spacingSmall

                        Repeater {
                            model: root.actorState ? root.actorState.aliases : []

                            delegate: Rectangle {
                                id: actorAliasChip
                                required property var modelData
                                required property int index
                                height: root.theme.viewAliasChipHeight
                                radius: root.theme.viewAliasChipRadius
                                color: root.actorState && root.actorState.isAliasSelected(actorAliasChip.index) ? root.theme.selectionHighlight : root.theme.surfaceAlt
                                border.width: 1
                                border.color: root.theme.border
                                width: Math.min(actorAliasFlow.width, actorAliasText.implicitWidth + root.theme.spacingLarge)

                                Text {
                                    id: actorAliasText
                                    anchors.centerIn: parent
                                    text: String(actorAliasChip.modelData)
                                    color: root.theme.textPrimary
                                    elide: Text.ElideRight
                                    width: parent.width - root.theme.spacing
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: if (root.actorState) root.actorState.selectAlias(actorAliasChip.index)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
