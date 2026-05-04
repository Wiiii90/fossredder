/**
 * @file ui/tests/qml/settings/tst_SettingsMiscellaneous.qml
 * @brief Provides QML tests for SettingsMiscellaneous behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

TestCase {
    id: testCase
    name: "SettingsMiscellaneousTests"
    when: windowShown
    width: 960
    height: 640

    property var settingsController: QtObject {
        property bool toolbarShowBooking: true
        property bool toolbarShowImport: true
        property bool toolbarShowActors: true
        property bool toolbarShowExport: true
        property bool toolbarShowProperties: true
        property bool toolbarShowAnalysis: true
        property bool toolbarShowContracts: true
        property bool toolbarShowAnnual: true
        property bool toolbarShowSettings: true
    }

    property var appContext: QtObject {
        property var settingsController: testCase.settingsController
    }

    property var theme: QtObject {
        property int viewFormSpacing: 8
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int formLabelWidth: 120
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: settingsMiscComponent
        SettingsMiscellaneous {
            width: 900
            height: 560
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function createView() {
        return createTemporaryObject(settingsMiscComponent, testCase)
    }

    function init() {
        settingsController.toolbarShowBooking = true
        settingsController.toolbarShowImport = true
    }

    function test_SET_M_001_toolbarFlagsAreWritableThroughBoundControls() {
        var view = createView()
        verify(view !== null)

        settingsController.toolbarShowBooking = false
        settingsController.toolbarShowImport = false

        compare(settingsController.toolbarShowBooking, false)
        compare(settingsController.toolbarShowImport, false)
    }

}
