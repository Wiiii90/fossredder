/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Property/PropertyView.qml
 * @brief Provides the PropertyView component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Property/PropertyView.qml
 * @brief Property page container that hosts the property form with app context and theme.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.PropertyForm { anchors.fill: parent; appContext: root.appContext; theme: root.theme }
}

