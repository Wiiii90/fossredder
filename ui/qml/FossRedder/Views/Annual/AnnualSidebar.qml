/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Annual/AnnualSidebar.qml
 * @brief Provides the AnnualSidebar component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Annual/AnnualSidebar.qml
 * @brief Sidebar list for navigating annual records.
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
            contentHeight: annualColumn.implicitHeight

            Column {
                id: annualColumn
                width: parent.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.session ? root.session.annuals : null

                    delegate: Rectangle {
                        id: annualRow
                        required property string id
                        required property string name
                        required property int year
                        width: annualColumn.width
                        height: 44
                        radius: 6
                        color: root.session && annualRow.id === root.session.selectedAnnualId
                               ? root.theme.selectionHighlight
                               : "transparent"
                        border.color: root.theme.borderSoft
                        border.width: root.theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.session)
                                    root.session.selectedAnnualId = annualRow.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: annualRow.name.length > 0
                                      ? annualRow.name
                                      : String(annualRow.year)
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: annualRow.year > 0 ? String(annualRow.year) : ""
                                color: root.theme.textMuted
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}

