import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"
import "qrc:/qml/components/common"

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

            // ensure delegate reports a consistent height so ListView can layout and
            // scrolling will always show the full box; include header + inner list height
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
                    // nicer toggle button using AppButton for consistent styling
                    AppButton {
                        id: toggleBtn
                        implicitWidth: 28; implicitHeight: 28
                        fillColor: "transparent"
                        textColor: Theme.textMuted
                        text: collapsed ? "\u25B6" : "\u25BC"
                        onClicked: collapsed = !collapsed
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                // header selection handled by headerMouse; toggleRect declared after so it receives clicks
            }

            ListView {
                id: txList
                width: statementList.width
                // use contentHeight so the parent delegate's implicitHeight can include it
                height: collapsed ? 0 : contentHeight
                visible: !collapsed
                interactive: false
                clip: true
                spacing: 2
                leftMargin: 14
                model: (uiData && statementId.length > 0) ? uiData.transactionsForStatement(statementId) : null

                delegate: ListRow {
                    // account for leftMargin so content does not overflow
                    width: statementList.width - 14
                    text: name ? name : ""
                    subtitle: bookingDate ? bookingDate : ""
                    selected: uiData ? (id === uiData.selectedTransactionId) : false
                    // ensure text eliding for long names
                    Component.onCompleted: {
                        // nothing here; ListRow labels already elide by default
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
