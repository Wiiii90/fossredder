import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: root

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: statementColumn.implicitHeight

        Column {
            id: statementColumn
            width: parent.width
            spacing: Theme.spacingSmall

            Repeater {
                model: session ? session.statementRows() : []

                delegate: Column {
                    width: statementColumn.width
                    property bool collapsed: false
                    property string statementId: (modelData.id !== undefined && modelData.id !== null) ? modelData.id : ""
                    property string statementName: (modelData.name !== undefined && modelData.name !== null) ? modelData.name : ""

                    Rectangle {
                        width: parent.width
                        height: 34
                        color: (session && statementId === session.selectedStatementId && (!session.selectedTransactionId || session.selectedTransactionId === ""))
                                   ? Theme.selectionHighlight : "transparent"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (!session) return
                                session.selectedStatementId = statementId
                                session.selectedTransactionId = ""
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.spacingSmall
                            Label { text: statementName; Layout.fillWidth: true; elide: Label.ElideRight }
                            Item { Layout.fillWidth: true }
                            Controls.Button {
                                implicitWidth: Theme.spacingLarge + Theme.margins * 4
                                implicitHeight: Theme.spacingLarge + Theme.margins * 4
                                fillColor: "transparent"
                                textColor: Theme.textMuted
                                text: collapsed ? "\u25B6" : "\u25BC"
                                onClicked: collapsed = !collapsed
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }

                    Column {
                        width: statementColumn.width - (Theme.spacing + Theme.margins)
                        leftPadding: Theme.spacing + Theme.margins
                        spacing: Theme.margins
                        visible: !collapsed

                        Repeater {
                            model: (session && statementId.length > 0) ? session.statementTransactionRows(statementId) : []

                            delegate: Rectangle {
                                width: statementColumn.width - (Theme.spacing + Theme.margins)
                                height: 40
                                radius: 6
                                color: session && modelData.id === session.selectedTransactionId ? Theme.selectionHighlight : "transparent"
                                border.color: Theme.borderSoft
                                border.width: Theme.borderWidthThin

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (!session) return
                                        session.selectedStatementId = statementId
                                        session.selectedTransactionId = modelData.id
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: Theme.spacingSmall
                                    spacing: Theme.spacingSmall

                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.name ? modelData.name : ""
                                        color: Theme.textPrimary
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        text: modelData.bookingDate ? modelData.bookingDate : ""
                                        color: Theme.textMuted
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
