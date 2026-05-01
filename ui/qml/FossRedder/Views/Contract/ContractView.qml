/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Contract/ContractView.qml
 * @brief Provides the ContractView component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Contract/ContractView.qml
 * @brief Contract page container that hosts the contract form with app context and theme.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.ContractForm { anchors.fill: parent; appContext: root.appContext; theme: root.theme }
}

