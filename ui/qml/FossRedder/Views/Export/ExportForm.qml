/**
 * @file ui/qml/FossRedder/Views/Export/ExportForm.qml
 * @brief Provides the ExportForm component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root
    required property var theme

    property string targetDirectory: ""
    property int packageFormatIndex: 0

    signal browseRequested()

    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        Label { text: qsTr("Target"); Layout.preferredWidth: root.theme.formLabelWidth }
        Controls.TextField {
            id: targetDirField
            objectName: "exportTargetDirectoryField"
            Layout.fillWidth: true
            placeholderText: qsTr("Select target directory...")
            text: root.targetDirectory
            onTextChanged: root.targetDirectory = text
        }
        Controls.SecondaryButton {
            objectName: "exportBrowseDirectoryButton"
            text: qsTr("Browse...")
            onClicked: root.browseRequested()
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Label { text: qsTr("Archive"); Layout.preferredWidth: root.theme.formLabelWidth }
        Controls.DropdownMenu {
            id: packageFormatBox
            objectName: "exportArchiveFormatComboBox"
            Layout.fillWidth: true
            model: [qsTr("None"), qsTr("ZIP")]
            currentIndex: root.packageFormatIndex
            onCurrentIndexChanged: root.packageFormatIndex = Math.min(currentIndex, 1)
        }
    }
}
