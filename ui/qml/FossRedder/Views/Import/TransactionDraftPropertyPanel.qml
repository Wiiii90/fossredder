import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Controls.Panel {
    id: root

    property var txRoot

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: Theme.spacingSmall

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingSmall

        Label { text: qsTr("Property"); Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Repeater {
                id: propertyRepeater
                model: session ? session.propertyRows() : []

                delegate: Item {
                    implicitHeight: row.implicitHeight
                    implicitWidth: row.implicitWidth

                    Row {
                        id: row
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: Theme.spacingSmall

                        Controls.CheckBox {
                            checked: root.txRoot && root.txRoot.draft && root.txRoot.draft.current && root.txRoot.draft.current.propertyIds && modelData && modelData.id
                                ? root.txRoot.draft.current.propertyIds.indexOf(modelData.id) !== -1
                                : false
                            onClicked: if (root.txRoot && root.txRoot.draft && draftController) {
                                draftController.setCurrentPropertySelected(root.txRoot.draft, modelData.id, checked)
                            }
                        }

                        Label {
                            text: modelData ? (modelData.display || modelData.name || "") : ""
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            Label {
                visible: propertyRepeater.count === 0
                text: qsTr("No properties available")
                color: Theme.textMuted
                Layout.fillWidth: true
            }
        }

        Label {
            text: root.txRoot ? root.txRoot.propertySuggestionSummary() : qsTr("No property suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : Theme.textMuted
            Layout.fillWidth: true
        }

    }
}
