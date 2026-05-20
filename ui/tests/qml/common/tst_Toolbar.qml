/**
 * @file ui/tests/qml/common/tst_Toolbar.qml
 * @brief Provides QML tests for Toolbar navigation side effects.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Components 1.0

TestCase {
    id: testCase
    name: "ToolbarTests"
    when: windowShown
    width: 960
    height: 180

    property var navigation: QtObject {
        property int sectionValue: -1
        function setSectionValue(value) { sectionValue = value }
    }

    property var session: QtObject {
        property string selectedActorId: ""
        property string selectedPropertyId: ""
        property string selectedContractId: ""
        property string selectedStatementId: ""
        property string selectedTransactionId: ""
        property string selectedAnalysisId: ""
        property string selectedAnnualId: ""
        function actorRows() { return [{ id: "actor-1" }, { id: "actor-2" }] }
        function propertyRows() { return [{ id: "property-1" }, { id: "property-2" }] }
        function contractRows() { return [{ id: "contract-1" }, { id: "contract-2" }] }
        function statementRows() { return [{ id: "statement-1" }, { id: "statement-2" }] }
        function statementTransactionRows(statementId) {
            return String(statementId || "") === "statement-1" ? [{ id: "tx-1" }] : []
        }
        function analysisRows() { return [{ id: "analysis-1" }, { id: "analysis-2" }] }
        function annualRows() { return [{ id: "annual-1" }, { id: "annual-2" }] }
    }

    property var appContext: QtObject {
        property var navigation: testCase.navigation
        property var session: testCase.session
        property var settingsViewModel: null
        property var importWorkflow: null
        property var actions: QtObject {}
        property var languageService: QtObject { property var availableLanguages: [] }
    }

    property var theme: QtObject {
        property int toolbarHeight: 96
        property int toolbarIconButtonWidth: 64
        property int toolbarSectionSpacing: 2
        property int toolbarGroupSpacing: 4
        property int spacing: 8
        property int spacingSmall: 6
        property int margins: 4
        property int borderWidthThin: 1
        property int radius: 3
        property int fontSizeSmall: 8
        property color toolbarBackground: "#ffffff"
        property color toolbarBorder: "#cccccc"
        property color divider: "#dddddd"
        property color textMuted: "#666666"
        property color textPrimary: "#000000"
        property color surface: "#ffffff"
        property color surfaceAlt: "#f5f5f5"
        property color border: "#cccccc"
        property color accent: "#3366ff"
    }

    Component {
        id: toolbarComponent
        Toolbar {
            width: 960
            height: 120
            appContext: testCase.appContext
            theme: testCase.theme
        }
    }

    Component {
        id: appMenuComponent
        AppMenu {
            appContext: testCase.appContext
            navigation: testCase.navigation
            actions: testCase.appContext.actions
            languageService: testCase.appContext.languageService
            settingsViewModel: null
            theme: testCase.theme
        }
    }

    function createToolbar() {
        return createTemporaryObject(toolbarComponent, testCase)
    }

    function createAppMenu() {
        return createTemporaryObject(appMenuComponent, testCase)
    }

    function init() {
        navigation.sectionValue = -1
        session.selectedActorId = ""
        session.selectedPropertyId = ""
        session.selectedContractId = ""
        session.selectedStatementId = ""
        session.selectedTransactionId = ""
        session.selectedAnalysisId = ""
        session.selectedAnnualId = ""
    }

    function test_domainNavigationOpensCreateMode() {
        var toolbar = createToolbar()
        session.selectedActorId = "actor-2"
        session.selectedPropertyId = "property-2"
        session.selectedContractId = "contract-2"

        toolbar.navigateTo(toolbar.navActors, true)
        compare(session.selectedActorId, "")

        toolbar.navigateTo(toolbar.navProperties, true)
        compare(session.selectedPropertyId, "")

        toolbar.navigateTo(toolbar.navContracts, true)
        compare(session.selectedContractId, "")
    }

    function test_bookingAndToolNavigationOpenCreateMode() {
        var toolbar = createToolbar()
        session.selectedStatementId = "statement-2"
        session.selectedTransactionId = "tx-2"
        session.selectedAnalysisId = "analysis-2"
        session.selectedAnnualId = "annual-2"

        toolbar.navigateToBookingCreate()
        compare(session.selectedStatementId, "")
        compare(session.selectedTransactionId, "")

        toolbar.navigateTo(toolbar.navAnalysis, true)
        compare(session.selectedAnalysisId, "")

        toolbar.navigateTo(toolbar.navAnnual, true)
        compare(session.selectedAnnualId, "")
    }

    function test_appMenuNavigationKeepsCreateMode() {
        var menu = createAppMenu()
        session.selectedActorId = "actor-2"
        session.selectedStatementId = "statement-2"
        session.selectedTransactionId = "tx-2"

        menu.navigateToSection(menu.navActors)
        compare(session.selectedActorId, "actor-2")

        menu.navigateToSection(menu.navBooking)
        compare(session.selectedStatementId, "statement-2")
        compare(session.selectedTransactionId, "tx-2")
    }
}
