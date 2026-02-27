import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls
import FossRedder.Components 1.0 as Components

Item {
    id: root

    ListView {
        id: statementList
        anchors.fill: parent
        clip: true
        spacing: Theme.spacingSmall
        model: uiData ? uiData.statements : null

        delegate: Column {
            width: statementList.width
            property bool collapsed: false

            property int headerHeight: 34
            height: headerHeight + (collapsed ? 0 : (txList ? txList.contentHeight : 0))

            property string statementId: (id !== undefined && id !== null) ? id : ""
            property string statementName: (name !== undefined && name !== null) ? name : ""

            Rectangle {
                id: headerRect
                width: parent.width
                height: headerHeight
                color: (uiData && statementId === uiData.selectedStatementId && (!uiData.selectedTransactionId || uiData.selectedTransactionId === ""))
                           ? "#ffd39c" : "transparent"

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
                    Controls.Button {
                        id: toggleBtn
                        implicitWidth: 28; implicitHeight: 28
                        fillColor: "transparent"
                        textColor: Theme.textMuted
                        text: collapsed ? "\u25B6" : "\u25BC"
                        onClicked: collapsed = !collapsed
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

            }

            ListView {
                id: txList
                width: statementList.width
                height: collapsed ? 0 : contentHeight
                visible: !collapsed
                interactive: false
                clip: true
                spacing: 2
                leftMargin: 14
                model: (uiData && statementId.length > 0) ? uiData.transactionsForStatement(statementId) : null

                delegate: Components.ListRow {
                    width: statementList.width - 14
                    text: name ? name : ""
                    subtitle: bookingDate ? bookingDate : ""
                    selected: uiData ? (id === uiData.selectedTransactionId) : false
                    Component.onCompleted: {
                    }
                    onActivated: {
                        if (!uiData) return
                        uiData.selectedStatementId = statementId
                        uiData.selectedTransactionId = id
                    }
                    height: 40
                }
            }
        }
    }
}
