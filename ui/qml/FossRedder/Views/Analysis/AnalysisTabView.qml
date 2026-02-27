import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views

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
        Views.AnalysisTabComponent {
            id: tabComp
            Layout.fillWidth: true
            Layout.fillHeight: true
            uiData: root.uiData
            uiDomain: root.uiDomain
        }

        Connections { target: (typeof uiData !== 'undefined') ? uiData : null
            function onLastAnalysisResultChanged() {
                try {
                    if (tabComp) { tabComp.uiData = uiData; if (tabComp.rebuild) tabComp.rebuild() }
                } catch(e) {}
            }
        }
    }
}

