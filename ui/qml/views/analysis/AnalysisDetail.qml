import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import components.controls 1.0
import "qrc:/qml/components/analysis"

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
            AppButton { text: qsTr("Back"); onClicked: {
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
                    source: (uiData && uiData.selectedAnalysisId) ? ((uiData.selectedAnalysis && uiData.selectedAnalysis.type === "calc") ? "qrc:/qml/views/analysis/AnalysisCalcView.qml" : ((uiData.selectedAnalysis && uiData.selectedAnalysis.type === "tab") ? "qrc:/qml/components/analysis/AnalysisTabComponent.qml" : "qrc:/qml/views/analysis/AnalysisPlotView.qml")) : "qrc:/qml/views/analysis/AnalysisPlotView.qml"
                    onLoaded: {
                        if (mainLoader.item) {
                            try { mainLoader.item.uiData = uiData } catch(e) {}
                            try { mainLoader.item.uiDomain = uiDomain } catch(e) {}
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
                    if (t === "calc") mainLoader.source = "qrc:/qml/views/analysis/AnalysisCalcView.qml"
                    else if (t === "tab") mainLoader.source = "qrc:/qml/components/analysis/AnalysisTabComponent.qml"
                    else mainLoader.source = "qrc:/qml/views/analysis/AnalysisPlotView.qml"
                } else {
                    mainLoader.source = "qrc:/qml/views/AnalysisPlotView.qml"
                }
                if (mainLoader.item && mainLoader.item.rebuild) mainLoader.item.rebuild()
            } catch(e) {}
        }
    }

    Timer { id: loaderInitTimer; interval: 60; repeat: false; running: false; triggeredOnStart: false; onTriggered: {
        try {
            if (!mainLoader || !mainLoader.item) return
            console.log('loaderInitTimer: reassign uiData to loaded item, uiData present=', !!uiData)
            try { mainLoader.item.uiData = uiData } catch(e) {}
            try { mainLoader.item.uiDomain = uiDomain } catch(e) {}
            try { if (mainLoader.item.rebuild) mainLoader.item.rebuild() } catch(e) {}
        } catch(e) {}
    } }
}
