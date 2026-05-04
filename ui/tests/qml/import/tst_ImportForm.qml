/**
 * @file ui/tests/qml/import/tst_ImportForm.qml
 * @brief Provides QML tests for ImportForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

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

        ImportForm {
            width: 600
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
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
