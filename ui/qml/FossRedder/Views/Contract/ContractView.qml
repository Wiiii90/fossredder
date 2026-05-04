/**
 * @file ui/qml/FossRedder/Views/Contract/ContractView.qml
 * @brief Provides the ContractView component.
 */

import QtQuick 2.15
import FossRedder.Views 1.0 as Views

Item {
    id: root
    required property var appContext
    required property var theme

    Views.ContractForm { anchors.fill: parent; appContext: root.appContext; theme: root.theme }
}

