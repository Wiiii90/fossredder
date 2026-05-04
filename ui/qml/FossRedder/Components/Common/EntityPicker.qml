/**
 * @file ui/qml/FossRedder/Components/Common/EntityPicker.qml
 * @brief Provides the EntityPicker component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property var actorController: root.appContext ? root.appContext.actorController : null
    readonly property var propertyController: root.appContext ? root.appContext.propertyController : null
    readonly property var contractController: root.appContext ? root.appContext.contractController : null
    property var model: null
    property var selectedIds: []
    property bool showAdd: true
    property string placeholder: qsTr("New item")
    property alias addEnabled: addField.text
    signal added(string id)

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(240, (root.model && (typeof root.model.count === 'function' ? root.model.count() : root.model.length)) * 42 + 8)
            contentHeight: delegateColumn.implicitHeight
            clip: true

            Column { id: delegateColumn; width: parent.width
                Repeater {
                    model: root.model
                    delegate: RowLayout { id: pickerRow
                        required property var model
                        Layout.fillWidth: true
                        spacing: 8
                        Controls.CheckBox {
                            Layout.preferredWidth: 28
                            checked: root.selectedIds.indexOf(pickerRow.model.id) !== -1
                            onClicked: {
                                let next = root.selectedIds ? root.selectedIds.slice(0) : []
                                const idx = next.indexOf(pickerRow.model.id)
                                if (checked) {
                                    if (idx === -1) next.push(pickerRow.model.id)
                                } else {
                                    if (idx !== -1) next.splice(idx, 1)
                                }
                                root.selectedIds = next
                            }
                        }
                        Label { text: pickerRow.model.name; Layout.fillWidth: true }
                    }
                }
            }
        }

        RowLayout {
            visible: root.showAdd
            Layout.fillWidth: true
            spacing: 6
            Controls.TextField { id: addField; objectName: "entityPickerAddField"; placeholderText: root.placeholder; Layout.fillWidth: true }
            Controls.Button {
                objectName: "entityPickerAddButton"
                text: qsTr("Add")
                enabled: addField.text && addField.text.length > 0
                onClicked: {
                    const name = addField.text.trim()
                    if (!name) return
                    let id = null
                    try {
                        if (root.actorController && root.session && root.model === root.session.actors) id = root.actorController.addActor(name, "", "")
                        else if (root.propertyController && root.session && root.model === root.session.properties) id = root.propertyController.addProperty(name, "", "")
                        else if (root.contractController && root.session && root.model === root.session.contracts) id = root.contractController.addContract(name, "", "", [], [])
                    } catch(e) { id = null }
                    if (id && id.length > 0) {
                        const next = root.selectedIds ? root.selectedIds.slice(0) : []
                        if (next.indexOf(id) === -1) next.push(id)
                        root.selectedIds = next
                        root.added(id)
                    }
                    addField.text = ""
                }
            }
        }
    }
}
