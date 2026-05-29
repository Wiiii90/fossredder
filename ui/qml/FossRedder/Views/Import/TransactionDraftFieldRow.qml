/**
 * @file ui/qml/FossRedder/Views/Import/TransactionDraftFieldRow.qml
 * @brief Provides a reusable two-column field row layout for transaction draft panels.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var theme

    property string leftLabel: ""
    property string rightLabel: ""
    property real leftWeight: 1
    property real rightWeight: 1
    property real columnSpacing: root.theme.spacingMedium
    property Component leftContent
    property Component rightContent

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(leftColumn.implicitHeight, rightColumn.implicitHeight)

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: root.columnSpacing

        ColumnLayout {
            id: leftColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.leftWeight
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingSmall

            Label { text: root.leftLabel; Layout.fillWidth: true }

            Loader {
                id: leftLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? Math.max(root.theme.controlHeight, item.implicitHeight || 0) : root.theme.controlHeight
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.leftContent
                onLoaded: if (item && item.hasOwnProperty("width")) item.width = width
                onWidthChanged: if (item && item.hasOwnProperty("width")) item.width = width
            }
        }

        ColumnLayout {
            id: rightColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.rightWeight
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingSmall

            Label { text: root.rightLabel; Layout.fillWidth: true }

            Loader {
                id: rightLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? Math.max(root.theme.controlHeight, item.implicitHeight || 0) : root.theme.controlHeight
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.rightContent
                onLoaded: if (item && item.hasOwnProperty("width")) item.width = width
                onWidthChanged: if (item && item.hasOwnProperty("width")) item.width = width
            }
        }
    }
}
