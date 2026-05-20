/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftMetadataPanel.qml
 * @brief Displays metadata preview fields for the current transaction draft.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    function currentText() {
        return metadataField.text
    }

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: Theme.spacingSmall

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Theme.spacingSmall

        Label { text: qsTr("Metadata"); Layout.fillWidth: true }
        Controls.TextArea {
            id: metadataField
            objectName: "transactionDraftMetadataTextArea"
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            Layout.minimumHeight: 0
            readOnly: true
            placeholderText: qsTr("Extracted metadata")
            text: root.txRoot && root.txRoot.draft && root.txRoot.draft.current ? (root.txRoot.draft.current.metadata || "") : ""
        }
    }
}
