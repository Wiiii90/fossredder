/**
 * @file ui/tests/qml/export/tst_ExportForm.qml
 * @brief Provides QML tests for ExportForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportFormTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
    }

    Component {
        id: exportFormComponent
        ExportForm {
            width: 700
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var found = Lookup.findObject(root, objectName)
        verify(found !== null, "Missing object: " + objectName)
        return found
    }

    function createForm() {
        return createTemporaryObject(exportFormComponent, testCase)
    }

    function test_EXP_F_001_targetDirectoryFieldUpdatesProperty() {
        var form = createForm()
        var targetField = findRequired(form, "exportTargetDirectoryField")

        targetField.text = "test:///export/target"

        compare(form.targetDirectory, "test:///export/target")
    }

    function test_EXP_F_002_archiveFormatDropdownUpdatesPackageIndex() {
        var form = createForm()
        var archiveCombo = findRequired(form, "exportArchiveFormatComboBox")

        archiveCombo.currentIndex = 1

        compare(form.packageFormatIndex, 1)
    }

    function test_EXP_F_003_browseButtonEmitsBrowseRequested() {
        var form = createForm()
        var browseButton = findRequired(form, "exportBrowseDirectoryButton")
        var emitted = false
        form.browseRequested.connect(function() { emitted = true })

        browseButton.clicked()

        compare(emitted, true)
    }

}
