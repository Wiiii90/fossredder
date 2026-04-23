/*!
 * @file ui/qml/FossRedder/Views/Contract/ContractActorsPanel.qml
 * @brief Actor selection panel for contracts with a single-select dropdown and a no-actor option.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    property var actorRows: []
    property var selectedActorIds: []
    signal selectionChanged(var ids)

    function actorDisplayModel() {
        const rows = root.actorRows || []
        const out = [{ id: "", display: qsTr("No actor") }]
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            out.push({
                id: row && row.id ? String(row.id) : "",
                display: row && row.name ? String(row.name) : ""
            })
        }
        return out
    }

    function selectedIndex() {
        const model = root.actorDisplayModel()
        if (!root.selectedActorIds || root.selectedActorIds.length === 0)
            return 0
        const selectedId = String(root.selectedActorIds[0])
        for (let i = 0; i < model.length; ++i) {
            const row = model[i]
            if (row && String(row.id) === selectedId)
                return i
        }
        return 0
    }

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
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Actor")
            Layout.fillWidth: true
        }

        Controls.ComboBox {
            id: actorCombo
            Layout.fillWidth: true
            textRole: "display"
            model: root.actorDisplayModel()
            currentIndex: root.selectedIndex()
            onActivated: {
                const row = model[currentIndex]
                root.selectionChanged(row && row.id ? [String(row.id)] : [])
            }
        }
    }
}
