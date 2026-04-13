import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    property var session: root.appContext ? root.appContext.session : null
    property var analysisController: root.appContext ? root.appContext.analysisController : null
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.fill: parent
    width: root.parent ? root.parent.width : 800
    height: root.parent ? root.parent.height : 600

    onSessionChanged: { try { if (tabComp) { tabComp.session = root.session; tabComp.analysisController = root.analysisController; if (tabComp.rebuild) tabComp.rebuild() } } catch(e) {} }

    ColumnLayout { anchors.fill: parent; spacing: 6
        Views.AnalysisTabComponent {
            id: tabComp
            Layout.fillWidth: true
            Layout.fillHeight: true
            session: root.session
            analysisController: root.analysisController
            appContext: root.appContext
            theme: root.theme
        }

        Connections { target: root.session
            function onLastAnalysisResultChanged() {
                try {
                    if (tabComp) { tabComp.session = root.session; if (tabComp.rebuild) tabComp.rebuild() }
                } catch(e) {}
            }
        }
    }
}

