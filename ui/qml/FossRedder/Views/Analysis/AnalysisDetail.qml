import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views

Item {
    id: page
    readonly property StateFacade session: AppContext.session
    readonly property AnalysisController analysisController: AppContext.analysisController
    Accessible.ignored: AppContext.isDebugBuild
    property var stackView
    property bool previewReady: false
    width: stackView ? stackView.width : (parent ? parent.width : Theme.analysis.layout.defaultWidth)
    height: stackView ? stackView.height : (parent ? parent.height : Theme.analysis.layout.defaultHeight)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pageMargin
        spacing: Theme.settings.spacing

        RowLayout { Layout.fillWidth: true
            Controls.Button { text: qsTr("Back"); onClicked: {
                    var sv = null
                    try { if (stackView && typeof stackView.pop === 'function') sv = stackView } catch(e) {}
                    try { if (!sv && page && page.parent && typeof page.parent.pop === 'function') sv = page.parent } catch(e) {}
                    try { if (sv) sv.pop() } catch(e) {}
                } }
            Item { Layout.fillWidth: true }
        }

        GroupBox { title: qsTr("Preview"); Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout { anchors.fill: parent; anchors.margins: Theme.chartPanelMargin; spacing: Theme.chartPanelSpacing
                Loader {
                    id: mainLoader
                    active: previewReady
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: (session && session.selectedAnalysisId)
                                     ? ((session.selectedAnalysis && session.selectedAnalysis.type === "calc") ? calcComp
                                        : ((session.selectedAnalysis && session.selectedAnalysis.type === "tab") ? tabComp
                                           : plotComp))
                                     : plotComp
                    onLoaded: {
                        if (mainLoader.item) {
                            try { mainLoader.item.session = session } catch(e) {}
                            try { mainLoader.item.analysisController = analysisController } catch(e) {}
                            try { if (mainLoader.item.rebuild) mainLoader.item.rebuild() } catch(e) {}
                            try { mainLoader.item.anchors.fill = mainLoader } catch(e) {}
                        }
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
        target: session
        function onSelectedAnalysisIdChanged() {
            try {
                if (!mainLoader) return
                if (session && session.selectedAnalysis) {
                    var t = session.selectedAnalysis.type ? session.selectedAnalysis.type : "plot"
                    if (t === "calc") mainLoader.sourceComponent = calcComp
                    else if (t === "tab") mainLoader.sourceComponent = tabComp
                    else mainLoader.sourceComponent = plotComp
                } else {
                    mainLoader.sourceComponent = plotComp
                }
                if (!previewReady) {
                    previewReady = true
                    Qt.callLater(function() {
                        if (mainLoader.item && mainLoader.item.rebuild) mainLoader.item.rebuild()
                    })
                    return
                }
                if (mainLoader.item && mainLoader.item.rebuild) mainLoader.item.rebuild()
            } catch(e) {}
        }
    }

    Component { id: calcComp; Views.AnalysisCalcView { } }
    Component { id: tabComp; Views.AnalysisTabComponent { } }
    Component { id: plotComp; Views.AnalysisPlotView { } }

    Timer { id: loaderInitTimer; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: {
        try {
            if (!mainLoader || !mainLoader.item) return
            try { mainLoader.item.session = session } catch(e) {}
            try { mainLoader.item.analysisController = analysisController } catch(e) {}
            try { if (mainLoader.item.rebuild) mainLoader.item.rebuild() } catch(e) {}
        } catch(e) {}
    } }

    Component.onCompleted: {
        Qt.callLater(function() {
            previewReady = true
            loaderInitTimer.restart()
        })
    }
}
