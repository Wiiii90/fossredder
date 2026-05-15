/**
 * @file ui/qml/FossRedder/Controls/FilePicker.qml
 * @brief Provides the FilePicker component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Popup {
    id: picker
    modal: true
    focus: true
    width: 600
    height: 400

    signal accepted(string path)
    signal rejected()

    property var fileSystemBrowser: null
    property string folder: picker.fileSystemBrowser ? picker.fileSystemBrowser.appDir() : (Qt.application ? Qt.resolvedUrl(".") : ".")
    property string selectedFile: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: folderField
                objectName: "filePickerFolderField"
                Layout.fillWidth: true
                text: picker.folder
                placeholderText: qsTr("Folder")
                onEditingFinished: picker.folder = text
            }
            Button {
                objectName: "filePickerUpButton"
                text: qsTr("Up")
                onClicked: {
                    let p = folderField.text
                    const idx = p.lastIndexOf(/[\\/]/)
                    if (idx > 0) {
                        p = p.substring(0, idx)
                        folderField.text = p
                        picker.folder = p
                    }
                }
            }
        }

        ListView {
            objectName: "filePickerEntryList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: picker.fileSystemBrowser ? picker.fileSystemBrowser.listDir(picker.folder) : []
            delegate: Item { id: fileRow
                required property var modelData
                width: parent.width
                height: 32
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text { text: fileRow.modelData.isDir ? "📁 " + fileRow.modelData.name : fileRow.modelData.name; elide: Text.ElideRight; Layout.fillWidth: true }
                    MouseArea {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: {
                            if (fileRow.modelData.isDir) { picker.folder = fileRow.modelData.path; folderField.text = fileRow.modelData.path }
                            else { picker.selectedFile = fileRow.modelData.path }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            TextField { id: nameField; objectName: "filePickerNameField"; Layout.fillWidth: true; placeholderText: qsTr("Filename (optional)") }
            Button {
                objectName: "filePickerSelectButton"
                text: qsTr("Select")
                onClicked: {
                    let out = nameField.text.trim()
                    if (out.length === 0) out = picker.selectedFile
                    else {
                        if (picker.folder.length > 0 && !picker.folder.endsWith("/") && !picker.folder.endsWith("\\")) out = picker.folder + "/" + out
                        else out = picker.folder + out
                    }
                    if (!out) return
                    picker.accepted(out)
                    picker.close()
                }
            }
            Button { objectName: "filePickerCancelButton"; text: qsTr("Cancel"); onClicked: { picker.rejected(); picker.close(); } }
        }
    }

    function open(dir) {
        if (dir) { picker.folder = dir; folderField.text = dir }
        picker.visible = true
    }

    Component.onCompleted: folderField.text = picker.folder
}
