/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Views/Booking/BookingStatementsSidebar.qml
 * @brief Provides the BookingStatementsSidebar component.
 */

/*!
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementsSidebar.qml
 * @brief Sidebar list of available statements in the booking page.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Controls 1.0 as Controls
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme

    readonly property var session: root.appContext ? root.appContext.session : null

    function selectedStatementIndex() {
        if (!root.session || !root.session.selectedStatementId) return -1
        const rows = root.session.statementRows()
        if (!rows || rows.length === undefined) return -1
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (row && row.id === root.session.selectedStatementId) return i
        }
        return -1
    }

    function ensureSelectedStatementVisible() {
        const idx = selectedStatementIndex()
        if (idx < 0 || !statementRepeater || !statementsFlick) return
        const item = statementRepeater.itemAt(idx)
        if (!item) return
        const top = item.y
        const bottom = top + item.height
        const viewportTop = statementsFlick.contentY
        const viewportBottom = viewportTop + statementsFlick.height
        if (top < viewportTop) {
            statementsFlick.contentY = top
        } else if (bottom > viewportBottom) {
            statementsFlick.contentY = Math.max(0, bottom - statementsFlick.height)
        }
    }

    Flickable {
        id: statementsFlick
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: statementColumn.implicitHeight

        Column {
            id: statementColumn
            width: parent.width
            spacing: root.theme.spacingSmall

            Repeater {
                id: statementRepeater
                model: root.session ? root.session.statementRows() : []

                delegate: Column {
                    id: statementEntry
                    required property var modelData
                    width: statementColumn.width
                    property bool collapsed: false
                    property string statementId: (statementEntry.modelData.id !== undefined && statementEntry.modelData.id !== null) ? statementEntry.modelData.id : ""
                    property string statementName: (statementEntry.modelData.name !== undefined && statementEntry.modelData.name !== null) ? statementEntry.modelData.name : ""

                    Rectangle {
                        width: parent.width
                        height: 34
                        color: (root.session && statementEntry.statementId === root.session.selectedStatementId && (!root.session.selectedTransactionId || root.session.selectedTransactionId === ""))
                                   ? root.theme.selectionHighlight : "transparent"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (!root.session) return
                                root.session.selectedStatementId = statementEntry.statementId
                                root.session.selectedTransactionId = ""
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: root.theme.spacingSmall
                            Label { text: statementEntry.statementName; Layout.fillWidth: true; elide: Label.ElideRight }
                            Item { Layout.fillWidth: true }
                            Controls.Button {
                                implicitWidth: root.theme.spacingLarge + root.theme.margins * 4
                                implicitHeight: root.theme.spacingLarge + root.theme.margins * 4
                                fillColor: "transparent"
                                textColor: root.theme.textMuted
                                text: statementEntry.collapsed ? "\u25B6" : "\u25BC"
                                onClicked: statementEntry.collapsed = !statementEntry.collapsed
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }

                    Column {
                        width: statementColumn.width - (root.theme.spacing + root.theme.margins)
                        leftPadding: root.theme.spacing + root.theme.margins
                        spacing: root.theme.margins
                        visible: !statementEntry.collapsed

                        Repeater {
                            model: (root.session && statementEntry.statementId.length > 0) ? root.session.statementTransactionRows(statementEntry.statementId) : []

                            delegate: Rectangle {
                                id: transactionEntry
                                required property var modelData
                                width: statementColumn.width - (root.theme.spacing + root.theme.margins)
                                height: 40
                                radius: 6
                                color: root.session && transactionEntry.modelData.id === root.session.selectedTransactionId ? root.theme.selectionHighlight : "transparent"
                                border.color: root.theme.borderSoft
                                border.width: root.theme.borderWidthThin

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (!root.session) return
                                        root.session.selectedStatementId = statementEntry.statementId
                                        root.session.selectedTransactionId = transactionEntry.modelData.id
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: root.theme.spacingSmall
                                    spacing: root.theme.spacingSmall

                                    Text {
                                        Layout.fillWidth: true
                                        text: transactionEntry.modelData.name ? transactionEntry.modelData.name : ""
                                        color: root.theme.textPrimary
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        text: transactionEntry.modelData.bookingDate ? transactionEntry.modelData.bookingDate : ""
                                        color: root.theme.textMuted
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

    Connections {
        target: root.session
        function onSelectedStatementIdChanged() { root.ensureSelectedStatementVisible() }
    }

    Connections {
        target: statementRepeater
        function onModelChanged() { root.ensureSelectedStatementVisible() }
    }
}
