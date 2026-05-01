/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Analysis/AnalysisView.qml
 * @brief Provides the AnalysisView component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    onVisibleChanged: {
        if (visible && analysisForm)
            analysisForm.refreshFromSelection()
    }

    Views.AnalysisForm {
        id: analysisForm
        anchors.fill: parent
        appContext: root.appContext
        theme: root.theme
    }
}
