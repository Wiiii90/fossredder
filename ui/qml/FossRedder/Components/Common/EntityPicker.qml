import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
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
            Layout.preferredHeight: Math.min(240, (model && (typeof model.count === 'function' ? model.count() : model.length)) * 42 + 8)
            contentHeight: delegateColumn.implicitHeight
            clip: true

            Column { id: delegateColumn; width: parent.width
                Repeater {
                    model: model
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Controls.CheckBox {
                            Layout.preferredWidth: 28
                            checked: root.selectedIds.indexOf(model.id) !== -1
                            onClicked: {
                                var next = root.selectedIds ? root.selectedIds.slice(0) : []
                                var idx = next.indexOf(model.id)
                                if (checked) {
                                    if (idx === -1) next.push(model.id)
                                } else {
                                    if (idx !== -1) next.splice(idx, 1)
                                }
                                root.selectedIds = next
                            }
                        }
                        Label { text: model.name; Layout.fillWidth: true }
                    }
                }
            }
        }

        RowLayout {
            visible: root.showAdd
            Layout.fillWidth: true
            spacing: 6
            Controls.TextField { id: addField; placeholderText: root.placeholder; Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Add")
                enabled: addField.text && addField.text.length > 0
                onClicked: {
                    if (!uiDomain) return
                    var name = addField.text.trim()
                    if (!name) return
                    var id = null
                    try {
                        if (typeof uiDomain.ensureActorByName === 'function') id = uiDomain.ensureActorByName(name)
                        else if (typeof uiDomain.ensurePropertyByName === 'function') id = uiDomain.ensurePropertyByName(name)
                        else if (typeof uiDomain.ensureContractByName === 'function') id = uiDomain.ensureContractByName(name)
                    } catch(e) { id = null }
                    if (id && id.length > 0) {
                        var next = root.selectedIds ? root.selectedIds.slice(0) : []
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
