import QtQuick 2.15
import QtTest 1.3
import FossRedder.Controls 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "FilePickerTests"
    when: windowShown
    width: 960
    height: 640

    property string lastAcceptedPath: ""

    property var fileSystemController: QtObject {
        property string appDirPath: "/workspace/app"
        property string lastListedPath: ""

        function reset() {
            appDirPath = "/workspace/app"
            lastListedPath = ""
        }

        function appDir() {
            return appDirPath
        }

        function listDir(path) {
            lastListedPath = path
            return []
        }
    }

    Component {
        id: filePickerComponent

        FilePicker {
            width: 640
            height: 480
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createPicker() {
        var picker = createTemporaryObject(filePickerComponent, testCase)
        picker.accepted.connect(function(path) {
            lastAcceptedPath = path
        })
        return picker
    }

    function init() {
        lastAcceptedPath = ""
        fileSystemController.reset()
    }

    function test_initialFolderUsesApplicationDirectory() {
        var picker = createPicker()
        var folderField = findRequired(picker, "filePickerFolderField")

        compare(picker.folder, "/workspace/app")
        compare(folderField.text, "/workspace/app")
    }

    function test_openUpdatesFolderAndField() {
        var picker = createPicker()
        var folderField = findRequired(picker, "filePickerFolderField")

        picker.open("/workspace/export")

        compare(picker.folder, "/workspace/export")
        compare(folderField.text, "/workspace/export")
    }

    function test_selectCombinesFolderAndTypedFilename() {
        var picker = createPicker()
        var nameField = findRequired(picker, "filePickerNameField")
        var selectButton = findRequired(picker, "filePickerSelectButton")

        picker.folder = "/workspace/export"
        nameField.text = "report.csv"
        selectButton.clicked()

        compare(lastAcceptedPath, "/workspace/export/report.csv")
    }

    function test_selectFallsBackToSelectedFileWhenNameIsEmpty() {
        var picker = createPicker()
        var nameField = findRequired(picker, "filePickerNameField")
        var selectButton = findRequired(picker, "filePickerSelectButton")

        picker.selectedFile = "/workspace/export/existing.csv"
        nameField.text = ""
        selectButton.clicked()

        compare(lastAcceptedPath, "/workspace/export/existing.csv")
    }
}
