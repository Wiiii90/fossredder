import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root
    required property var theme
    required property var propertyRows
    property var selectedIds: []
    signal selectionChanged(var ids)

    Layout.fillWidth: true
    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
    Layout.preferredHeight: root.theme.viewSelectionPanelPreferredHeight
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: root.theme.spacingSmall

        Label {
            text: qsTr("Properties")
            Layout.fillWidth: true
        }

        Flickable {
            id: propertyScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: propertyColumn.implicitHeight

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            Column {
                id: propertyColumn
                width: propertyScroll.width
                spacing: root.theme.spacingSmall

                Repeater {
                    model: root.propertyRows
                    delegate: RowLayout {
                        id: rowRoot
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: root.theme.spacingSmall

                        Controls.CheckBox {
                            Layout.fillWidth: false
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            checked: root.selectedIds.indexOf(rowRoot.modelData.id) !== -1
                            onClicked: {
                                const next = root.selectedIds ? root.selectedIds.slice() : []
                                const idx = next.indexOf(rowRoot.modelData.id)
                                if (checked && idx === -1)
                                    next.push(rowRoot.modelData.id)
                                if (!checked && idx !== -1)
                                    next.splice(idx, 1)
                                root.selectionChanged(next)
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            text: rowRoot.modelData && rowRoot.modelData.name ? rowRoot.modelData.name : ""
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
