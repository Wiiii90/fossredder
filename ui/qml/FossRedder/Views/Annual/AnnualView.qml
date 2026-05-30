/**
 * @file ui/qml/FossRedder/Views/Annual/AnnualView.qml
 * @brief Provides the Annual view composition.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Annual 1.0 as Annual
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var annualState
    required property var theme

    onVisibleChanged: {
        if (visible)
            root.annualState.refreshFromSelection()
    }

    ColumnLayout {
        anchors.fill: root
        anchors.margins: root.theme.pageContentMargin
        spacing: root.theme.spacingSmall

        Annual.AnnualForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: root.theme
            annualState: root.annualState
        }

        Annual.AnnualBottomBar {
            Layout.fillWidth: true
            theme: root.theme
            annualState: root.annualState
        }
    }
}
