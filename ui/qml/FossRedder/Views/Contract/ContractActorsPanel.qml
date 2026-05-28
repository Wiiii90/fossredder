/**
 * @file ui/qml/FossRedder/Views/Contract/ContractActorsPanel.qml
 * @brief Provides the ContractActorsPanel component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var sessionState
    required property var contractState
    property var actorRows: []

    Layout.fillWidth: true
    Layout.preferredWidth: 1
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
            text: qsTr("Actor")
            Layout.fillWidth: true
        }

        Controls.DropdownMenu {
            id: actorCombo
            objectName: "contractActorComboBox"
            readonly property string selectedActorId: root.contractState && root.contractState.selectedActorIds.length > 0
                                                     ? String(root.contractState.selectedActorIds[0])
                                                     : ""
            Layout.fillWidth: true
            textRole: "display"
            model: root.sessionState ? root.sessionState.displayRowsWithEmpty(root.actorRows || [], qsTr("No actor"), "name") : []
            currentIndex: {
                const idx = root.sessionState ? root.sessionState.indexOfId(model, selectedActorId) : -1
                return idx >= 0 ? idx : 0
            }
            onActivated: function(index) {
                const row = model[index]
                if (root.contractState)
                    root.contractState.selectPrimaryActor(row && row.id ? String(row.id) : "")
            }
        }
    }
}
