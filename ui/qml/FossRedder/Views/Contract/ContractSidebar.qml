/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Contract/ContractSidebar.qml
 * @brief Provides the ContractSidebar component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Contract/ContractSidebar.qml
 * @brief Sidebar list for navigating available contract records.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.theme.spacingMedium
        spacing: root.theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractColumn.implicitHeight

            Column {
                id: contractColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.session ? root.session.contractRows() : []

                    delegate: Rectangle {
                        id: contractRow
                        required property var modelData
                        width: contractColumn.width
                        height: 44
                        radius: 6
                        color: root.session && contractRow.modelData.id === root.session.selectedContractId ? root.theme.selectionHighlight : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session) root.session.selectedContractId = contractRow.modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: contractRow.modelData.name ? contractRow.modelData.name : ""
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}

