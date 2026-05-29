/**
 * @file ui/tests/qml/import/tst_ImportForm.qml
 * @brief Provides QML tests for ImportForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views.Import 1.0 as Import

import "../Lookup.js" as Lookup
import "../TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ImportFormTests"
    when: windowShown
    width: 960
    height: 640

    property var theme: QtObject {
        property int spacingSmall: 6
        property int formLabelWidth: 120
    }

    Component {
        id: importFormComponent

        Import.ImportForm {
            width: 600
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createForm() {
        return createTemporaryObject(importFormComponent, testCase)
    }

    function test_IMP_F_001_sourceSelectorShowsSupportedLabels() {
        var form = createForm()
        var sourceCombo = findRequired(form, "importSourceComboBox")

        verify(sourceCombo.model !== null)
        verify(sourceCombo.model.length > 0)
    }

    function test_IMP_F_002_strategySelectorShowsSupportedLabels() {
        var form = createForm()
        var strategyCombo = findRequired(form, "importStrategyComboBox")

        verify(strategyCombo.model !== null)
        verify(strategyCombo.model.length > 0)
    }

}
