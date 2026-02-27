import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Views 1.0 as Views

Item {
    id: page
    property var stackView
    width: stackView ? stackView.width : (parent ? parent.width : 800)
    height: stackView ? stackView.height : (parent ? parent.height : 600)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Component.onCompleted: { console.log("AnalysisDetail: page width=", page.width, "height=", page.height) }

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
            ColumnLayout { anchors.fill: parent; anchors.margins: 6; spacing: 6
                Loader {
                    id: mainLoader
                    active: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: (uiData && uiData.selectedAnalysisId)
                                     ? ((uiData.selectedAnalysis && uiData.selectedAnalysis.type === "calc") ? calcComp
                                        : ((uiData.selectedAnalysis && uiData.selectedAnalysis.type === "tab") ? tabComp
                                           : plotComp))
                                     : plotComp
                    onLoaded: {
                        if (mainLoader.item) {
                            try { mainLoader.item.uiData = uiData } catch(e) {}
                            try { mainLoader.item.analysisController = analysisController } catch(e) {}
                            try { if (mainLoader.item.rebuild) mainLoader.item.rebuild() } catch(e) {}
                            try { mainLoader.item.anchors.fill = mainLoader } catch(e) {}
                        }
                    }
                    onStatusChanged: {
                        try {
                            if (mainLoader.status === Loader.Error) {
                                console.log("AnalysisDetail: Loader error loading", mainLoader.source, "errorString:", mainLoader.errorString)
                            }
                        } catch(e) {}
                    }
                }
            }
        }
    }
    Connections {
        target: typeof uiData !== 'undefined' ? uiData : null
        function onSelectedAnalysisIdChanged() {
            try {
                if (!mainLoader) return
                if (typeof uiData !== 'undefined' && uiData && uiData.selectedAnalysis) {
                    var t = uiData.selectedAnalysis.type ? uiData.selectedAnalysis.type : "plot"
                    if (t === "calc") mainLoader.sourceComponent = calcComp
                    else if (t === "tab") mainLoader.sourceComponent = tabComp
                    else mainLoader.sourceComponent = plotComp
                } else {
                    mainLoader.sourceComponent = plotComp
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
            console.log('loaderInitTimer: reassign uiData to loaded item, uiData present=', !!uiData)
            try { mainLoader.item.uiData = uiData } catch(e) {}
            try { mainLoader.item.analysisController = analysisController } catch(e) {}
            try { if (mainLoader.item.rebuild) mainLoader.item.rebuild() } catch(e) {}
        } catch(e) {}
    } }
}
