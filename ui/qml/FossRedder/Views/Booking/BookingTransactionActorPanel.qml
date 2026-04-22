import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme
    required property var txRoot

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
            model: root.txRoot.actorDisplayModel()
            currentIndex: root.txRoot.selectedIndexFor(model, root.txRoot.actorIdValue())
            onActivated: {
                const row = model[currentIndex]
                root.txRoot.updateField("actorId", row && row.id ? row.id : "")
            }
        }
    }
}
