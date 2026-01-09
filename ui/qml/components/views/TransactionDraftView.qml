import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls" as Controls
import "qrc:/qml/components/utils/FileUtils.js" as FileUtils
import FossRedder 1.0

Item {
    id: txRoot
    // anchors.fill: parent

    property var draft
    function formatAmount(a) {
        if (a === undefined || a === null) return "";
        try {
            var v = Number(a);
            if (!isFinite(v)) return "";
            var s = v.toFixed(2); // always two decimals
            // use comma as decimal separator for display
            s = s.replace('.', ',');
            return s;
        } catch (e) { return String(a); }
    }
    implicitHeight: txLayout.implicitHeight
    implicitWidth: txLayout.implicitWidth

    ColumnLayout {
        id: txLayout
        width: txRoot.width

        RowLayout {
            Layout.fillWidth: true

            Label { text: qsTr("Name"); Layout.preferredWidth: 80 }

            Controls.AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? (draft.current.name || "") : ""
                onTextChanged: if (draft) draft.transactions.setName(draft.currentIndex, text)
            }
        }

        Loader {
            id: proofLoader
            Layout.fillWidth: true
            Layout.preferredHeight: item ? item.implicitHeight : 0
            active: !!draft && !!draft.current && (draft.current.proofImagePath || "").length > 0

            property string _lastPath: ""

            function resetIfNeeded() {
                var p = (draft && draft.current) ? (draft.current.proofImagePath || "") : ""
                if (p === _lastPath) return
                _lastPath = p
                active = false
                Qt.callLater(function() { proofLoader.active = p.length > 0 })
            }

            sourceComponent: Item {
                width: proofLoader.width
                implicitHeight: proofImage.status === Image.Ready ? proofImage.implicitHeight : 0

                Image {
                    id: proofImage
                    width: parent.width
                    fillMode: Image.PreserveAspectFit
                    source: FileUtils.toFileUrl((draft && draft.current) ? (draft.current.proofImagePath || "") : "")
                    cache: true
                    asynchronous: true
                }
            }

            Component.onCompleted: resetIfNeeded()
        }

        RowLayout {
            Layout.fillWidth: true

            Label { text: qsTr("Booking date"); Layout.alignment: Qt.AlignVCenter }
            Controls.AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? (draft.current.bookingDate || "") : ""
                onTextChanged: if (draft) draft.transactions.setBookingDate(draft.currentIndex, text)
            }

            Label { text: qsTr("Amount"); Layout.alignment: Qt.AlignVCenter }
            Controls.AppTextField {
                Layout.fillWidth: true
                text: draft && draft.current ? formatAmount(draft.current.amount) : ""
                onTextChanged: {
                    if (!draft) return
                    var v = parseFloat(text)
                    if (isNaN(v)) v = 0.0
                    draft.transactions.setAmount(draft.currentIndex, v)
                }
            }
        }

        Label {
            text: qsTr("Metadata")
            font.bold: true
        }

        Controls.AppTextArea {
            Layout.fillWidth: true
            wrapMode: TextArea.Wrap
            text: draft && draft.current ? (draft.current.metadata || "") : ""
            selectByMouse: true
            onTextChanged: if (draft) draft.transactions.setMetadata(draft.currentIndex, text)
        }

        // Properties section grouped visually
        GroupBox {
            title: qsTr("Properties")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                // keep the list compact but allow it to grow with content
                ListView {
                    id: propertyListView
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: contentHeight
                    interactive: false
                    model: uiData ? uiData.properties : null

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Layout.alignment: Qt.AlignVCenter

                        Controls.AppCheckBox {
                            id: propCheck
                            Layout.preferredWidth: 28
                            Layout.margins: 2
                             // reflect whether this property id is assigned to the draft transaction
                             checked: (draft && draft.current && draft.current.propertyIds && model.id) ? draft.current.propertyIds.indexOf(model.id) !== -1 : false
                             onCheckedChanged: {
                                 if (!draft || !draft.current || !model.id) return
                                 var ids = draft.current.propertyIds ? draft.current.propertyIds.slice() : []
                                 var idx = ids.indexOf(model.id)
                                 if (checked) {
                                     if (idx === -1) ids.push(model.id)
                                 } else {
                                     if (idx > -1) ids.splice(idx, 1)
                                 }
                                 draft.transactions.setProperties(draft.currentIndex, ids)
                             }
                         }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: model.name }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Label {
                    id: propEmptyLabel
                    Layout.fillWidth: true
                    visible: propertyListView.count === 0
                    text: qsTr("No properties available")
                }
            }
        } // close GroupBox

        // Allocatable checkbox kept visually separate from properties
        RowLayout {
            Layout.fillWidth: true
            Controls.AppCheckBox {
                id: allocCheck_local
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Allocatable to tenant")
                checked: draft && draft.current ? !!draft.current.allocatable : false
                onCheckedChanged: if (draft) draft.transactions.setAllocatable(draft.currentIndex, checked)
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Status"); Layout.preferredWidth: 80 }
            Controls.AppComboBox {
                id: statusCombo_local
                Layout.fillWidth: true
                model: [ qsTr("Neutral"), qsTr("Unverified"), qsTr("Verified"), qsTr("Completed") ]
                currentIndex: draft && draft.current ? Math.max(0, draft.current.status) : 2
                onActivated: if (draft) draft.transactions.setStatus(draft.currentIndex, currentIndex)
            }
            Item { Layout.fillWidth: true }
        }
    }
    function forceSync() { }
}
