import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    ListView {
        id: statementList
        anchors.fill: parent
        clip: true
        spacing: 8
        model: uiData ? uiData.statements : null

        delegate: Column {
            width: statementList.width
            property bool collapsed: false

            property string statementId: (id !== undefined && id !== null) ? id : ""
            property string statementName: (name !== undefined && name !== null) ? name : ""

            Rectangle {
                width: parent.width
                height: 34
                color: (uiData && statementId === uiData.selectedStatementId && (!uiData.selectedTransactionId || uiData.selectedTransactionId === ""))
                           ? "#ffd39c" : "transparent"

                // header selection area underneath; toggle button is declared after so it receives clicks
                MouseArea {
                    id: headerMouse
                    anchors.fill: parent
                    onClicked: {
                        if (!uiData) return
                        uiData.selectedStatementId = statementId
                        uiData.selectedTransactionId = ""
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    Label { text: statementName; Layout.fillWidth: true; elide: Label.ElideRight }
                    Item { Layout.fillWidth: true }
                    // simple chevron toggle (no animation) placed last so it is on top and captures clicks
                    Rectangle {
                        id: toggleRect
                        width: 24; height: 24; radius: 4
                        color: "transparent"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: collapsed ? "\u25B6" : "\u25BC" // ▶ / ▼
                            font.pointSize: 12
                            color: "#666"
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: collapsed = !collapsed
                        }
                    }
                }

                // header selection area (does not overlap toggleRect)
                MouseArea {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: toggleRect.left
                    onClicked: {
                        if (!uiData) return
                        uiData.selectedStatementId = statementId
                        uiData.selectedTransactionId = ""
                    }
                }
            }

            ListView {
                width: statementList.width
                height: collapsed ? 0 : contentHeight
                visible: !collapsed
                interactive: false
                clip: true
                spacing: 2
                leftMargin: 14
                model: (uiData && statementId.length > 0) ? uiData.transactionsForStatement(statementId) : null

                delegate: Rectangle {
                    width: statementList.width
                    height: 30
                    color: (uiData && id === uiData.selectedTransactionId) ? "#ffd39c" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6

                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: (status === 1) ? "#e74c3c" : ((status === 2) ? "#f1c40f" : ((status === 3) ? "#2ecc71" : "#9e9e9e"))
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Label {
                            Layout.fillWidth: true
                            text: name
                            elide: Label.ElideRight
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!uiData) return
                            uiData.selectedStatementId = statementId
                            uiData.selectedTransactionId = id
                        }
                    }
                }
            }
        }
    }
}
