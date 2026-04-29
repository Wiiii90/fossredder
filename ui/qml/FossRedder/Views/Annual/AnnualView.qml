/*!
 * @file ui/qml/FossRedder/Views/Annual/AnnualView.qml
 * @brief Annual page container that hosts the annual form with app context and theme.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.AnnualForm {
        anchors.fill: parent
        appContext: root.appContext
        theme: root.theme
    }
}
