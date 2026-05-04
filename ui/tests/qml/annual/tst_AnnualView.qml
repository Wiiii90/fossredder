/**
 * @file ui/tests/qml/annual/tst_AnnualView.qml
 * @brief Provides QML tests for AnnualView behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "AnnualViewTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedAnnual: null
        property string selectedAnnualId: ""
        property var annuals: []
        property var analysesData: []

        function annualRows() { return annuals || [] }
        function analysisRows() { return analysesData || [] }
        function navigatedId(rows, currentId, delta, fallbackIndex) {
            var list = rows || []
            if (list.length === 0)
                return ""
            var idx = 0
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(currentId || "")) {
                    idx = i
                    break
                }
            }
            idx = (idx + delta + list.length) % list.length
            return String(list[idx].id || "")
        }
        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) { return "" }
    }

    property var annualController: QtObject {
        function annual(id) { return ({ id: "", name: "", year: 0, transactionIds: [], assignedAnalysisIds: [] }) }
        function saveAnnual(id, name, year, assignedAnalysisIds) { return id && id.length > 0 ? id : "annual-new" }
        function deleteAnnual(id) {}
    }

    property var analysisController: QtObject {
        function updateAnalysis(id, name, type, config, filter, exportFormat, includeCalcAdjustments, exportState, snapshotTransactions) {}
    }

    property var transactionController: QtObject {
        function transaction(id) { return ({}) }
        function transactions() { return [] }
    }

    property var appContext: QtObject {
        property var session: testCase.session
        property var annualController: testCase.annualController
        property var analysisController: testCase.analysisController
        property var transactionController: testCase.transactionController
        property bool isDebugBuild: false
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int spacingSmall: 6
        property int spacing: 8
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int controlHeight: 32
        property int viewCompactActionButtonSize: 28
        property int viewActionButtonWidth: 120
        property int radius: 3
        property int borderWidthThin: 1
        property color borderSoft: "#cccccc"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property color textMuted: "#666666"
    }

    Component {
        id: annualViewComponent
        AnnualView {
            width: 960
            height: 640
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        var match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createView() {
        return createTemporaryObject(annualViewComponent, testCase)
    }

    function init() {
        session.selectedAnnualId = ""
        session.selectedAnnual = null
        session.annuals = []
        session.analysesData = []
    }

    function test_mountsAnnualFormCreateMode() {
        var view = createView()
        var nameField = findRequired(view, "annualNameField")
        verify(nameField !== null)
    }

    function test_mountsAnnualFormEditMode() {
        session.selectedAnnualId = "annual-1"
        session.selectedAnnual = Qt.createQmlObject('import QtQml 2.15; QtObject { property string id: "annual-1" }', testCase)

        var view = createView()
        var updateButton = findRequired(view, "annualUpdateButton")
        verify(updateButton !== null)
    }
}
