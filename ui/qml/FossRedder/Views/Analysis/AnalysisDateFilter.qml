/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisDateFilter.qml
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
    property int dateFieldIndex: 0
    property int dateModeIndex: 0
    property string yearValue: ""
    property string dateFromValue: ""
    property string dateToValue: ""
    property bool suppressFilterChanged: false
    readonly property real modeFieldWidth: root.theme.formFieldWidth
    readonly property real valueFieldWidth: root.theme.formFieldWidth
    readonly property real yearFieldWidth: (root.theme.formFieldWidth * 2) + root.theme.spacingSmall + implicitFromLabel.implicitWidth + implicitToLabel.implicitWidth
    signal filterChanged()

    Layout.fillWidth: true
    contentSpacing: root.theme.spacingSmall

    RowLayout {
        Layout.fillWidth: true
        spacing: root.theme.spacingSmall

        Controls.DropdownMenu {
            id: dateFieldCombo
            objectName: "analysisDateFieldComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [ qsTr("Booking Date"), qsTr("Valuta") ]
            currentIndex: root.dateFieldIndex
            onCurrentIndexChanged: {
                root.dateFieldIndex = currentIndex
                if (!root.suppressFilterChanged)
                    root.filterChanged()
            }
        }

        Controls.DropdownMenu {
            id: dateModeCombo
            objectName: "analysisDateModeComboBox"
            Layout.preferredWidth: root.theme.formFieldWidth
            model: [ qsTr("Year"), qsTr("Date Range") ]
            currentIndex: root.dateModeIndex
            onCurrentIndexChanged: {
                root.dateModeIndex = currentIndex
                if (!root.suppressFilterChanged)
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
            objectName: "analysisDateFromField"
            visible: root.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.dateFromValue
            onTextChanged: {
                root.dateFromValue = text
                if (!root.suppressFilterChanged)
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
            objectName: "analysisDateToField"
            visible: root.dateModeIndex === 1
            Layout.preferredWidth: root.theme.formFieldWidth
            placeholderText: qsTr("YYYY-MM-DD")
            text: root.dateToValue
            onTextChanged: {
                root.dateToValue = text
                if (!root.suppressFilterChanged)
                    root.filterChanged()
            }
        }

        Controls.TextField {
            id: yearField
            objectName: "analysisYearField"
            visible: root.dateModeIndex === 0
            Layout.preferredWidth: root.yearFieldWidth
            placeholderText: qsTr("YYYY")
            text: root.yearValue
            onTextChanged: {
                root.yearValue = text
                if (!root.suppressFilterChanged)
                    root.filterChanged()
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
