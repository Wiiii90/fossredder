/**
 * @file ui/tests/qml/property/tst_PropertyView.qml
 * @brief Provides QML tests for PropertyView behavior.
 */

import QtQuick 2.15
import QtTest 1.3

TestCase {
    id: testCase
    name: "PropertyViewTests"
    when: windowShown
    width: 960
    height: 640

    function test_smoke_suiteIsWired() {
        verify(true)
    }
}
