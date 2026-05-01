/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Analysis/AnalysisDateFilter.qml
 * @brief Provides the AnalysisDateFilter component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    property int dateModeIndex: 1
    property string yearValue: ""
    property string dateFromValue: ""
    property string dateToValue: ""
    readonly property real modeFieldWidth: root.theme.formFieldWidth
    readonly property real valueFieldWidth: root.theme.formFieldWidth
    readonly property real yearFieldWidth: (root.theme.formFieldWidth * 2) + root.theme.spacingSmall + implicitFromLabel.implicitWidth + implicitToLabel.implicitWidth
    signal filterChanged()

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Date")
            Layout.preferredWidth: root.theme.formLabelWidth
        }

        Controls.DropdownMenu {
            id: dateModeCombo
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [ qsTr("Year"), qsTr("Date Range") ]
            currentIndex: root.dateModeIndex
            onCurrentIndexChanged: {
                root.dateModeIndex = currentIndex
                root.filterChanged()
            }
        }

        Label {
            id: implicitFromLabel
            visible: root.dateModeIndex === 1
            text: qsTr("From")
        }

        Controls.TextField {
            id: dateFromField
            visible: root.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.dateFromValue
            onTextChanged: {
                root.dateFromValue = text
                root.filterChanged()
            }
        }

        Label {
            id: implicitToLabel
            visible: root.dateModeIndex === 1
            text: qsTr("To")
        }

        Controls.TextField {
            id: dateToField
            visible: root.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.dateToValue
            onTextChanged: {
                root.dateToValue = text
                root.filterChanged()
            }
        }

        Controls.TextField {
            id: yearField
            visible: root.dateModeIndex === 0
            Layout.preferredWidth: root.yearFieldWidth
            placeholderText: qsTr("YYYY")
            text: root.yearValue
            onTextChanged: {
                root.yearValue = text
                root.filterChanged()
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
