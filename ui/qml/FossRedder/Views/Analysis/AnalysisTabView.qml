import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views

Item {
    id: root
    property StateFacade session: AppContext.session
    property AnalysisController analysisController: AppContext.analysisController
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    onSessionChanged: { try { if (tabComp) { tabComp.session = session; tabComp.analysisController = analysisController; if (tabComp.rebuild) tabComp.rebuild() } } catch(e) {} }

    ColumnLayout { anchors.fill: parent; spacing: 6
        Views.AnalysisTabComponent {
            id: tabComp
            Layout.fillWidth: true
            Layout.fillHeight: true
            session: root.session
            analysisController: root.analysisController
        }

        Connections { target: session
            function onLastAnalysisResultChanged() {
                try {
                    if (tabComp) { tabComp.session = session; if (tabComp.rebuild) tabComp.rebuild() }
                } catch(e) {}
            }
        }
    }
}

