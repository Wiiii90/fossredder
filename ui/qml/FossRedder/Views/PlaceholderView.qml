/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/PlaceholderView.qml
 * @brief Provides the PlaceholderView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import FossRedder 1.0

Item {
    Rectangle {
        anchors.fill: parent
        color: Theme.background
        Label { anchors.centerIn: parent; text: qsTr("Not implemented") }
    }
}
