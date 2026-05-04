/**
 * @file ui/tests/qml/analysis/tst_AnalysisPlotView.qml
 * @brief Provides QML tests for AnalysisPlotView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnalysisPlotViewTests"
    when: windowShown
    width: 960
    height: 640

    property string exportState: "{}"
    property string emittedExportState: ""

    property var session: QtObject {
        property var lastAnalysisResult: ({
            table: [
                ["Lease", "100.0"],
                ["Service", "50.0"]
            ]
        })
        function propertyName(id) { return id }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property bool isDebugBuild: false
    }

    property var theme: QtObject {
        property int spacingSmall: 6
        property int spacing: 8
        property int spacingMedium: 10
        property int radius: 3
        property int borderWidthThin: 1
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color borderLight: "#d7d7d7"
        property color borderStrong: "#888888"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
        property color chartFallback: "#77aadd"
        property var analysis: QtObject {
            property var palette: ["#77aadd", "#88ccaa", "#ddaa77", "#cc88aa"]
            property var layout: QtObject {
                property int splitControlsWidth: 180
            }
            property var table: QtObject {
                property int propertyColumnWidth: 180
                property int amountColumnWidth: 120
                property int totalColumnWidth: 140
                property int rowHeight: 30
            }
        }
        property int chartPlotMinimumHeight: 180
        property int chartPlotPreferredHeight: 260
        property int chartLegendHeight: 80
        property int chartLegendMarkerSize: 12
        property int chartValueLabelWidth: 70
        property int chartPercentLabelWidth: 58
        property string fontFamily: "Segoe UI"
        property int fontSize: 10
    }

    Component {
        id: plotViewComponent
        AnalysisPlotView {
            width: 960
            height: 640
            appContext: testCase.appContext
            theme: testCase.theme
            exportStateJson: testCase.exportState
            onExportStateChanged: (stateJson) => testCase.emittedExportState = stateJson
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(plotViewComponent, testCase)
    }

    function init() {
        exportState = "{}"
        emittedExportState = ""
    }

    function test_histogramSplitSwitchPersistsExportState() {
        var view = createView()
        var splitSwitch = findRequired(view, "analysisHistogramSplitSwitch")

        splitSwitch.checked = true
        verify(emittedExportState.indexOf("histogramSplitByProperty") !== -1)
    }
}
