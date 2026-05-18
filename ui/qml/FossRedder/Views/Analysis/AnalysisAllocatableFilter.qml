/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisAllocatableFilter.qml
 * @brief Provides the AnalysisAllocatableFilter component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property string mode: "all"
    property bool initialized: false
    signal modeSelected(string mode)

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Allocatable")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.DropdownMenu {
            id: allocatableCombo
            objectName: "analysisAllocatableModeComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [ qsTr("All"), qsTr("Only allocatable"), qsTr("Only non allocatable") ]
            currentIndex: root.mode === "allocatable" ? 1 : (root.mode === "non-allocatable" ? 2 : 0)
            onCurrentIndexChanged: {
                if (!root.initialized)
                    return
                const next = currentIndex === 1 ? "allocatable" : (currentIndex === 2 ? "non-allocatable" : "all")
                root.modeSelected(next)
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: root.initialized = true
}
