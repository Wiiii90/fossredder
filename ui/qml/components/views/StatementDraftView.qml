import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "qrc:/qml/components/controls"
import FossRedder 1.0

Item {
    id: stmtRoot
    property var draft
    anchors.fill: parent
    Layout.fillWidth: true
    Layout.fillHeight: true

    property bool hasProperties: (uiData && uiData.properties && uiData.properties.length > 0)

    ColumnLayout {
        id: stmtLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 10
        Layout.fillWidth: true
        Layout.fillHeight: true

        // If no draft, show a placeholder message
        Label {
            visible: !draft
            text: qsTr("No draft available")
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
        }

        // Rest of UI only shown when draft exists
        Item {
            visible: !!draft
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 0
                spacing: 10
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("Name"); Layout.preferredWidth: 80 }
                    AppTextField { Layout.fillWidth: true; text: draft ? draft.name : ""; onTextChanged: if (draft) draft.name = text }
                }

                Label {
                    Layout.fillWidth: true
                    text: (draft && draft.current) ? (qsTr("Transaction %1 / %2").arg(draft.currentIndex + 1).arg(draft.count)) : qsTr("No current transaction")
                }

                GroupBox {
                    title: qsTr("Transaction")
                    Layout.fillWidth: true
                    TransactionDraftView { id: txView; Layout.fillWidth: true }
                }

                GroupBox {
                    id: propsGroup
                    title: qsTr("Properties")
                    Layout.fillWidth: true

                    property var selectedWeights: ({ })
                    property var allocShares: ({ })

                    function refreshPropertyUi() {
                        propsGroup.selectedWeights = { };
                        propsGroup.allocShares = { };
                        if (!draft || !draft.current) return;
                        var allocs = draft.current.propertyAllocations || [];
                        for (var i = 0; i < allocs.length; ++i) {
                            var a = allocs[i];
                            if (!a.propertyId) continue;
                            propsGroup.selectedWeights[a.propertyId] = { selected: true, weight: (a.weight || 1.0) };
                        }
                        updateAllocShares();
                    }

                    function updateAllocShares() {
                        var total = 0.0;
                        for (var k in propsGroup.selectedWeights) { if (propsGroup.selectedWeights[k] && propsGroup.selectedWeights[k].selected) total += Number(propsGroup.selectedWeights[k].weight) || 0.0 }
                        propsGroup.allocShares = { };
                        if (total <= 0.0) return;
                        for (var k in propsGroup.selectedWeights) {
                            if (!propsGroup.selectedWeights[k] || !propsGroup.selectedWeights[k].selected) continue;
                            propsGroup.allocShares[k] = (Number(propsGroup.selectedWeights[k].weight) || 0.0) / total;
                        }
                    }

                    function updateAllocationsModel() {
                        var list = [];
                        for (var k in propsGroup.selectedWeights) {
                            if (!propsGroup.selectedWeights[k] || !propsGroup.selectedWeights[k].selected) continue;
                            list.push({ propertyId: k, weight: Number(propsGroup.selectedWeights[k].weight) || 1.0 });
                        }
                        if (draft) draft.transactions.setPropertyAllocations(draft.currentIndex, list);
                        updateAllocShares();
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6
                        Layout.fillWidth: true

                        RowLayout {
                            Layout.fillWidth: true
                            Button {
                                text: qsTr("Auto-assign")
                                enabled: !!draft && !!uiDomain && hasProperties
                                onClicked: {
                                    if (!draft || !uiDomain) return
                                    try { uiDomain.autoAssignPropertiesForDraft(draft) } catch (e) {}
                                    Qt.callLater(function() { if (propsGroup) propsGroup.refreshPropertyUi && propsGroup.refreshPropertyUi() })
                                }
                            }
                            Item { Layout.fillWidth: true }
                        }

                        ListView {
                            id: propListView
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            model: hasProperties ? uiData.properties : []
                            delegate: Item {
                                implicitHeight: 36
                                width: parent.width
                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 8
                                    Layout.fillWidth: true
                                    CheckBox {
                                        id: selBox
                                        checked: (propsGroup.selectedWeights[model.id] && propsGroup.selectedWeights[model.id].selected) ? propsGroup.selectedWeights[model.id].selected : false
                                        onCheckedChanged: {
                                            if (!propsGroup.selectedWeights[model.id]) propsGroup.selectedWeights[model.id] = { selected: false, weight: 1.0 }
                                            propsGroup.selectedWeights[model.id].selected = checked
                                            if (!propsGroup.selectedWeights[model.id].weight) propsGroup.selectedWeights[model.id].weight = 1.0
                                            updateAllocationsModel()
                                        }
                                    }
                                    Label { text: model.name; Layout.preferredWidth: 240 }
                                    TextField {
                                        id: weightField
                                        enabled: selBox.checked
                                        text: (propsGroup.selectedWeights[model.id] && propsGroup.selectedWeights[model.id].weight) ? String(propsGroup.selectedWeights[model.id].weight) : "1"
                                        onEditingFinished: {
                                            var v = parseFloat(text); if (isNaN(v) || v <= 0) v = 1.0; text = String(v)
                                            if (!propsGroup.selectedWeights[model.id]) propsGroup.selectedWeights[model.id] = { selected: selBox.checked, weight: v }
                                            propsGroup.selectedWeights[model.id].weight = v
                                            updateAllocationsModel()
                                        }
                                        Layout.preferredWidth: 80
                                    }
                                    Label {
                                        text: (propsGroup.allocShares[model.id] ? Math.round(propsGroup.allocShares[model.id]*1000)/10 + "%" : "")
                                        Layout.alignment: Qt.AlignVCenter
                                        Layout.preferredWidth: 80
                                    }
                                    Item { Layout.fillWidth: true }
                                }
                            }
                        }
                    }

                    Component.onCompleted: refreshPropertyUi()
                }

                RowLayout {
                    Layout.fillWidth: true
                    Button { text: qsTr("Prev"); enabled: draft && draft.currentIndex > 0; onClicked: { if (draft) draft.prev(); txView.forceSync && txView.forceSync() } }
                    Button { text: qsTr("Next"); enabled: draft && (draft.currentIndex < draft.count - 1); onClicked: { if (draft) draft.next(); txView.forceSync && txView.forceSync() } }
                    Item { Layout.fillWidth: true }
                    Button { text: qsTr("Finish"); enabled: !!draft; onClicked: { if (!draft) return; if (typeof uiDomain !== 'undefined' && uiDomain) { var sid = uiDomain.finalizeStatementDraft(draft); if (sid && sid.length > 0 && uiNav && uiData) { uiNav.section = UiNavigation.Booking; uiNav.bookingView = UiNavigation.Statements; uiData.selectedStatementId = sid; uiData.selectedTransactionId = "" } return } } }
                }
            }
        }
    }

    Component.onCompleted: { if (txView && draft) txView.draft = draft; Qt.callLater(function() { if (propsGroup) propsGroup.refreshPropertyUi && propsGroup.refreshPropertyUi() }) }

    Connections { target: draft; function onChanged() { if (txView) txView.draft = draft; Qt.callLater(function() { if (propsGroup) propsGroup.refreshPropertyUi && propsGroup.refreshPropertyUi() }) } }

    function forceRefreshProperties() { stmtRoot.forceRefreshProperties = null /*no-op marker*/ }
}