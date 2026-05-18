/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisView.qml
 * @brief Provides the AnalysisView component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    function scheduleRefresh() {
        Qt.callLater(function() {
            if (analysisForm)
                analysisForm.refreshFromSelection()
        })
    }

    onVisibleChanged: {
        if (visible && analysisForm)
            root.scheduleRefresh()
    }

    Connections {
        target: root.session
        function onSelectedAnalysisIdChanged() {
            if (analysisForm)
                root.scheduleRefresh()
        }
        function onDataRevisionChanged() {
            if (analysisForm)
                root.scheduleRefresh()
        }
    }

    Views.AnalysisForm {
        id: analysisForm
        anchors.fill: parent
        appContext: root.appContext
        theme: root.theme
        Component.onCompleted: root.scheduleRefresh()
    }
}
