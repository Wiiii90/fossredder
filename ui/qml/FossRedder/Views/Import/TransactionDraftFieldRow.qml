import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root

    property string leftLabel: ""
    property string rightLabel: ""
    property real leftWeight: 1
    property real rightWeight: 1
    property Component leftContent
    property Component rightContent

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(leftColumn.implicitHeight, rightColumn.implicitHeight)

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.top: parent.top
        spacing: Theme.spacingMedium

        ColumnLayout {
            id: leftColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.leftWeight
            Layout.alignment: Qt.AlignTop
            spacing: Theme.spacingSmall

            Label { text: root.leftLabel; Layout.fillWidth: true }

            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: item ? Math.max(40, item.implicitHeight) : 40
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.leftContent
                onLoaded: {
                    if (item && item.hasOwnProperty("width")) item.width = width
                }
                onWidthChanged: if (item && item.hasOwnProperty("width")) item.width = width
            }
        }

        ColumnLayout {
            id: rightColumn
            Layout.fillWidth: true
            Layout.preferredWidth: root.rightWeight
            Layout.alignment: Qt.AlignTop
            spacing: Theme.spacingSmall

            Label { text: root.rightLabel; Layout.fillWidth: true }

            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: item ? Math.max(40, item.implicitHeight) : 40
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.rightContent
                onLoaded: {
                    if (item && item.hasOwnProperty("width")) item.width = width
                }
                onWidthChanged: if (item && item.hasOwnProperty("width")) item.width = width
            }
        }
    }
}
