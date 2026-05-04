/**
 * @file ui/qml/FossRedder/Views/Actor/ActorView.qml
 * @brief Provides the ActorView component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.ActorForm { anchors.fill: parent; appContext: root.appContext; theme: root.theme }
}
