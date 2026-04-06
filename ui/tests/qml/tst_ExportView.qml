import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "ExportViewTests"
    when: windowShown
    width: 960
    height: 640

    property var exportController: QtObject {
        property int callCount: 0
        property var lastCall: ({})

        function reset() {
            callCount = 0
            lastCall = ({})
        }

        function exportData(format, path, includeFormulas, locale) {
            callCount += 1
            lastCall = {
                format: format,
                path: path,
                includeFormulas: includeFormulas,
                locale: locale
            }
        }
    }

    property var fileSystemController: QtObject {
        function appDir() {
            return "/workspace/out"
        }
    }

    property var actions: QtObject {
        signal exportFileSelected(string path)
        property int browseCalls: 0

        function reset() {
            browseCalls = 0
        }

        function browseExportFile() {
            browseCalls += 1
        }
    }

    Component {
        id: exportViewComponent

        ExportView {
            width: 960
            height: 640
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(exportViewComponent, testCase)
    }

    function init() {
        exportController.reset()
        actions.reset()
    }

    function test_formatSelectionBuildsDefaultPath() {
        var view = createView()
        var formatBox = findRequired(view, "exportFormatBox")
        var pathField = findRequired(view, "exportPathField")

        pathField.text = ""
        formatBox.currentIndex = 1
        compare(pathField.text, "/workspace/out/export.xlsx")

        formatBox.currentIndex = 0
        compare(pathField.text, "/workspace/out/export.csv")
    }

    function test_exportButtonForwardsSelectedArguments() {
        var view = createView()
        var formatBox = findRequired(view, "exportFormatBox")
        var formulas = findRequired(view, "exportIncludeFormulasCheckBox")
        var localeField = findRequired(view, "exportLocaleField")
        var pathField = findRequired(view, "exportPathField")
        var exportButton = findRequired(view, "exportSubmitButton")

        formatBox.currentIndex = 1
        formulas.checked = false
        localeField.text = "de-DE"
        pathField.text = "/workspace/out/report.xlsx"
        exportButton.clicked()

        compare(exportController.callCount, 1)
        compare(exportController.lastCall.format, view.formatOptions[1].contract)
        compare(exportController.lastCall.path, "/workspace/out/report.xlsx")
        compare(exportController.lastCall.includeFormulas, false)
        compare(exportController.lastCall.locale, "de-DE")
    }

    function test_browseAndSelectedPathFlowThroughUiActions() {
        var view = createView()
        var browseButton = findRequired(view, "exportBrowseButton")
        var pathField = findRequired(view, "exportPathField")

        browseButton.clicked()
        compare(actions.browseCalls, 1)

        actions.exportFileSelected("/picked/from/dialog.csv")
        compare(pathField.text, "/picked/from/dialog.csv")
    }
}
