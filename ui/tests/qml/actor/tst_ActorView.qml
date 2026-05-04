/**
 * @file ui/tests/qml/actor/tst_ActorView.qml
 * @brief Provides QML tests for ActorView behavior.
 */

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

TestCase {
    id: testCase
    name: "ActorViewTests"
    when: windowShown
    width: 960
    height: 640

    function test_smoke_suiteIsWired() {
        verify(true)
    }
}
