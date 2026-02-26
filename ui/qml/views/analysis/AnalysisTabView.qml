import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/analysis"

Item {
    id: root
    property var uiData
    property var uiDomain
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    onUiDataChanged: { try { if (tabComp) { tabComp.uiData = uiData; tabComp.uiDomain = uiDomain; if (tabComp.rebuild) tabComp.rebuild() } } catch(e) {} }

    ColumnLayout { anchors.fill: parent; spacing: 6
        Loader {
            id: tabLoader
            active: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "qrc:/qml/components/analysis/AnalysisTabComponent.qml"
            onLoaded: {
                if (tabLoader.item) {
                    try { tabLoader.item.uiData = uiData } catch(e) {}
                    try { tabLoader.item.uiDomain = uiDomain } catch(e) {}
                    try { if (tabLoader.item.rebuild) tabLoader.item.rebuild() } catch(e) {}
                }
            }
        }

        Connections { target: (typeof uiData !== 'undefined') ? uiData : null
            function onLastAnalysisResultChanged() {
                try {
                    if (tabLoader.item) { tabLoader.item.uiData = uiData; if (tabLoader.item.rebuild) tabLoader.item.rebuild() }
                    else if (uiData && !tabLoader.active) tabLoader.active = true
                } catch(e) {}
            }
        }
    }
}
