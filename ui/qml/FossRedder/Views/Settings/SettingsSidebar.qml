/**
 * @file ui/qml/FossRedder/Views/Settings/SettingsSidebar.qml
 * @brief Renders settings category navigation entries in the sidebar.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var navigation: root.appContext ? root.appContext.navigation : null

    readonly property var categories: [
        { cat: 0, text: qsTr("General") },
        { cat: 1, text: qsTr("Import") },
        { cat: 2, text: qsTr("Export") },
        { cat: 3, text: qsTr("Miscellaneous") }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: settingsColumn.implicitHeight

            Column {
                id: settingsColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.categories

                    delegate: Rectangle {
                        id: settingsRow
                        required property var modelData
                        width: settingsColumn.width
                        height: 44
                        radius: 6
                        color: root.navigation && settingsRow.modelData.cat === root.navigation.settingsCategoryValue ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.navigation) root.navigation.setSettingsCategoryValue(settingsRow.modelData.cat)
                            }
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.leftMargin: root.theme.spacingSmall
                            anchors.topMargin: root.theme.spacingSmall
                            width: parent.width - (2 * root.theme.spacingSmall)
                            text: settingsRow.modelData.text
                            color: root.theme.textPrimary
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}

