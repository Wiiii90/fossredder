/**
 * @file ui/qml/FossRedder/Views/Import/ImportProgressBar.qml
 * @brief Provides import progress feedback.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var importState

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    Controls.ProgressBar {
        objectName: "importProgressBar"
        Layout.fillWidth: true
        visible: true
        value: root.importState.progressValue
    }

    Label {
        Layout.fillWidth: true
        text: root.importState.progressText
        color: root.importState.progressHasError ? root.theme.danger : root.theme.textPrimary
        wrapMode: Text.WordWrap
    }
}
