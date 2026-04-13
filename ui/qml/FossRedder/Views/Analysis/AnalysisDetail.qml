import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: page
    required property var appContext
    required property var theme
    readonly property var session: page.appContext ? page.appContext.session : null
    readonly property var analysisController: page.appContext ? page.appContext.analysisController : null
    Accessible.ignored: page.appContext ? page.appContext.isDebugBuild : false
    property var stackView
    property bool previewReady: false
    readonly property string sessionMemberName: "session"
    readonly property string analysisControllerMemberName: "analysisController"
    readonly property string rebuildMemberName: "rebuild"
    readonly property string anchorsMemberName: "anchors"
    width: page.stackView ? page.stackView.width : (parent ? parent.width : page.theme.analysis.layout.defaultWidth)
    height: page.stackView ? page.stackView.height : (parent ? parent.height : page.theme.analysis.layout.defaultHeight)

    function tryPopCurrentStack() {
        try {
            const popFn = page.stackView ? page.stackView["pop"] : null
            if (typeof popFn === "function") popFn.call(page.stackView)
        } catch (e) {
        }
    }

    function tryBindLoaderItem(item) {
        try {
            if (!item) return
            try { item[page.sessionMemberName] = page.session } catch (e) {}
            try { item[page.analysisControllerMemberName] = page.analysisController } catch (e) {}
            try {
                const rebuildFn = item[page.rebuildMemberName]
                if (typeof rebuildFn === "function") rebuildFn.call(item)
            } catch (e) {}
            try {
                const anchorsObj = item[page.anchorsMemberName]
                if (anchorsObj) anchorsObj.fill = mainLoader
            } catch (e) {}
        } catch (e) {
        }
    }

    function tryRebuildLoaderItem() {
        try {
            const item = mainLoader ? mainLoader.item : null
            if (!item) return
            const rebuildFn = item[page.rebuildMemberName]
            if (typeof rebuildFn === "function") rebuildFn.call(item)
        } catch (e) {
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: page.theme.pageMargin
        spacing: page.theme.settings.spacing

        RowLayout { Layout.fillWidth: true
            Controls.Button { text: qsTr("Back"); onClicked: {
                    page.tryPopCurrentStack()
                } }
            Item { Layout.fillWidth: true }
        }

        GroupBox { title: qsTr("Preview"); Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout { anchors.fill: parent; anchors.margins: page.theme.chartPanelMargin; spacing: page.theme.chartPanelSpacing
                Loader {
                    id: mainLoader
                    active: page.previewReady
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: (page.session && page.session.selectedAnalysisId)
                                     ? ((page.session.selectedAnalysis && page.session.selectedAnalysis.type === "calc") ? calcComp
                                        : ((page.session.selectedAnalysis && page.session.selectedAnalysis.type === "tab") ? tabComp
                                           : plotComp))
                                     : plotComp
                    onLoaded: {
                        page.tryBindLoaderItem(mainLoader.item)
                    }
                    onStatusChanged: {
                        try {
                            if (mainLoader.status === Loader.Error) {}
                        } catch(e) {}
                    }
                }
            }
        }
    }
    Connections {
        target: page.session
        function onSelectedAnalysisIdChanged() {
            try {
                if (!mainLoader) return
                if (page.session && page.session.selectedAnalysis) {
                    const t = page.session.selectedAnalysis.type ? page.session.selectedAnalysis.type : "plot"
                    if (t === "calc") mainLoader.sourceComponent = calcComp
                    else if (t === "tab") mainLoader.sourceComponent = tabComp
                    else mainLoader.sourceComponent = plotComp
                } else {
                    mainLoader.sourceComponent = plotComp
                }
                if (!page.previewReady) {
                    page.previewReady = true
                    Qt.callLater(function() {
                        page.tryRebuildLoaderItem()
                    })
                    return
                }
                page.tryRebuildLoaderItem()
            } catch(e) {}
        }
    }

    Component { id: calcComp; Views.AnalysisCalcView { appContext: page.appContext; theme: page.theme } }
    Component { id: tabComp; Views.AnalysisTabComponent { appContext: page.appContext; theme: page.theme } }
    Component { id: plotComp; Views.AnalysisPlotView { appContext: page.appContext; theme: page.theme } }

    Timer { id: loaderInitTimer; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: {
        try {
            page.tryBindLoaderItem(mainLoader ? mainLoader.item : null)
        } catch(e) {}
    } }

    Component.onCompleted: {
        Qt.callLater(function() {
            page.previewReady = true
            loaderInitTimer.restart()
        })
    }
}
