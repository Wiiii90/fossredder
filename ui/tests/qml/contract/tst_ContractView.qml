/**
 * @file ui/tests/qml/contract/tst_ContractView.qml
 * @brief Provides QML tests for ContractView behavior.
 */

import QtQuick 2.15
import QtTest 1.3

TestCase {
    id: testCase
    name: "ContractViewTests"
    when: windowShown
    width: 960
    height: 640

    function test_smoke_suiteIsWired() {
        verify(true)
    }
}
