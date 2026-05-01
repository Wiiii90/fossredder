/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Import/TransactionDraftFieldRow.qml
 * @brief Provides the TransactionDraftFieldRow component.
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
    property Component leftContent
    property Component rightContent

    function loaderItem(loader) {
        return loader ? loader["item"] : null
    }

    function loaderPreferredHeight(loader) {
        const loadedItem = root.loaderItem(loader)
        return loadedItem ? Math.max(40, loadedItem.implicitHeight || 0) : 40
    }

    function syncLoaderWidth(loader) {
        const loadedItem = root.loaderItem(loader)
        if (loadedItem && loadedItem.hasOwnProperty("width")) loadedItem.width = loader.width
    }

    function onLoaderWidthChanged(loader) {
        root.syncLoaderWidth(loader)
    }

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(leftColumn.implicitHeight, rightColumn.implicitHeight)

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.top: parent.top
        spacing: root.theme.spacingMedium

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
                Layout.preferredHeight: root.loaderPreferredHeight(leftLoader)
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.leftContent
                onLoaded: root.syncLoaderWidth(leftLoader)
                onWidthChanged: root.onLoaderWidthChanged(leftLoader)
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
                Layout.preferredHeight: root.loaderPreferredHeight(rightLoader)
                Layout.alignment: Qt.AlignTop
                sourceComponent: root.rightContent
                onLoaded: root.syncLoaderWidth(rightLoader)
                onWidthChanged: root.onLoaderWidthChanged(rightLoader)
            }
        }
    }
}
