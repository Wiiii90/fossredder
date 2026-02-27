import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Popup {
    id: picker
    modal: true
    focus: true
    width: 600
    height: 400

    signal accepted(string path)
    signal rejected()

    property string folder: (typeof fileSystemController !== 'undefined' && fileSystemController) ? fileSystemController.appDir() : (Qt.application ? Qt.resolvedUrl(".") : ".")
    property string selectedFile: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: folderField
                Layout.fillWidth: true
                text: picker.folder
                placeholderText: qsTr("Folder")
                onEditingFinished: picker.folder = text
            }
            Button {
                text: qsTr("Up")
                onClicked: {
                    var p = folderField.text
                    var idx = p.lastIndexOf(/[\\/]/)
                    if (idx > 0) {
                        p = p.substring(0, idx)
                        folderField.text = p
                        picker.folder = p
                    }
                }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: (typeof fileSystemController !== 'undefined' && fileSystemController) ? fileSystemController.listDir(picker.folder) : []
            delegate: Item {
                width: parent.width
                height: 32
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text { text: modelData.isDir ? "📁 " + modelData.name : modelData.name; elide: Text.ElideRight; Layout.fillWidth: true }
                    MouseArea {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: {
                            if (modelData.isDir) { picker.folder = modelData.path; folderField.text = modelData.path }
                            else { picker.selectedFile = modelData.path }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            TextField { id: nameField; Layout.fillWidth: true; placeholderText: qsTr("Filename (optional)") }
            Button {
                text: qsTr("Select")
                onClicked: {
                    var out = nameField.text.trim()
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
            Button { text: qsTr("Cancel"); onClicked: { picker.rejected(); picker.close(); } }
        }
    }

    function open(dir) {
        if (dir) { picker.folder = dir; folderField.text = dir }
        picker.open()
    }

    Component.onCompleted: folderField.text = picker.folder
}
