import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Controls.Panel {
    id: root

    property var txRoot
    required property var appContext
    required property var theme
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var draftController: root.appContext ? root.appContext.draftController : null

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    contentSpacing: root.theme.spacingSmall

    background: Rectangle {
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : root.theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Label { text: qsTr("Property"); Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: root.theme.spacingSmall

            Repeater {
                id: propertyRepeater
                model: root.session ? root.session.propertyRows() : []

                delegate: Item {
                    id: propertyOption
                    required property var modelData
                    implicitHeight: row.implicitHeight
                    implicitWidth: row.implicitWidth

                    Row {
                        id: row
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: root.theme.spacingSmall

                        Controls.CheckBox {
                            checked: root.txRoot && root.txRoot.draft && root.txRoot.draft.current && root.txRoot.draft.current.propertyIds && propertyOption.modelData && propertyOption.modelData.id
                                ? root.txRoot.draft.current.propertyIds.indexOf(propertyOption.modelData.id) !== -1
                                : false
                            onClicked: if (root.txRoot && root.txRoot.draft && root.draftController) {
                                root.draftController.setCurrentPropertySelected(root.txRoot.draft, propertyOption.modelData.id, checked)
                            }
                        }

                        Label {
                            text: propertyOption.modelData ? (propertyOption.modelData.display || propertyOption.modelData.name || "") : ""
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            Label {
                visible: propertyRepeater.count === 0
                text: qsTr("No properties available")
                color: root.theme.textMuted
                Layout.fillWidth: true
            }
        }

        Label {
            text: root.txRoot ? root.txRoot.propertySuggestionSummary() : qsTr("No property suggestion")
            color: root.txRoot ? root.txRoot.suggestionColor(root.txRoot.propertyTopSuggestion()) : root.theme.textMuted
            Layout.fillWidth: true
        }

    }
}
