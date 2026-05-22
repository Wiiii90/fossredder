/**
 * @file ui/qml/FossRedder/Views/Booking/BookingStatementsSidebar.qml
 * @brief Provides the BookingStatementsSidebar component.
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
    readonly property int workspaceRevision: root.session ? root.session.dataRevision : 0
    readonly property real rowRadius: (root.theme && root.theme.viewSidebarRowRadius !== undefined && root.theme.viewSidebarRowRadius !== null) ? Number(root.theme.viewSidebarRowRadius) : Number(root.theme.radius || 3)
    readonly property real rowHeight: (root.theme && root.theme.viewSidebarRowHeight !== undefined && root.theme.viewSidebarRowHeight !== null) ? Number(root.theme.viewSidebarRowHeight) : 36
    readonly property color panelSurfaceAlt: (root.theme && root.theme.surfaceAlt !== undefined && root.theme.surfaceAlt !== null) ? root.theme.surfaceAlt : "#f5f5f5"

    function statementRows() {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.statementRows() : []
    }

    function statementTransactionRows(statementId) {
        const _workspaceRevision = root.workspaceRevision
        return root.session ? root.session.statementTransactionRows(statementId) : []
    }

    function selectedStatementIndex() {
        const _workspaceRevision = root.workspaceRevision
        if (!root.session || !root.session.selectedStatementId) return -1
        const rows = root.statementRows()
        if (!rows || rows.length === undefined) return -1
        for (let i = 0; i < rows.length; ++i) {
            const row = rows[i]
            if (row && row.id === root.session.selectedStatementId) return i
        }
        return -1
    }

    function maxContentY() {
        return Math.max(0, statementsFlick.contentHeight - statementsFlick.height)
    }

    function scrollItemToTop(item) {
        if (!item || !statementsFlick || !statementColumn) return
        const point = item.mapToItem(statementColumn, 0, 0)
        statementsFlick.contentY = Math.max(0, Math.min(point.y, root.maxContentY()))
    }

    function selectedStatementItem() {
        const idx = selectedStatementIndex()
        if (idx < 0 || !statementRepeater) return null
        const item = statementRepeater.itemAt(idx)
        return item ? item.statementRowItem() : null
    }

    function selectedTransactionItem() {
        if (!root.session || !root.session.selectedTransactionId) return null
        for (let i = 0; i < statementRepeater.count; ++i) {
            const item = statementRepeater.itemAt(i)
            if (!item) continue
            const txItem = item.transactionItemById(root.session.selectedTransactionId)
            if (txItem) return txItem
        }
        return null
    }

    function scrollSelectionToTop() {
        const txItem = selectedTransactionItem()
        if (txItem) {
            root.scrollItemToTop(txItem)
            return
        }
        root.scrollItemToTop(selectedStatementItem())
    }

    Flickable {
        id: statementsFlick
        objectName: "bookingStatementsFlick"
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
                model: root.statementRows()

                delegate: Rectangle {
                    id: statementEntry
                    objectName: "bookingStatementRow_" + statementEntry.statementId
                    required property var modelData
                    width: statementColumn.width
                    property bool collapsed: false
                    property string statementId: (statementEntry.modelData.id !== undefined && statementEntry.modelData.id !== null) ? statementEntry.modelData.id : ""
                    property string statementName: (statementEntry.modelData.name !== undefined && statementEntry.modelData.name !== null) ? statementEntry.modelData.name : ""
                    readonly property bool isSelectedStatement: root.session && statementEntry.statementId === root.session.selectedStatementId
                    color: root.panelSurfaceAlt
                    radius: root.rowRadius
                    border.width: root.theme.borderWidthThin
                    border.color: statementEntry.isSelectedStatement ? root.theme.selectionHighlight : root.theme.borderSoft
                    implicitHeight: statementContent.implicitHeight + (root.theme.spacingSmall * 2)
                    function statementRowItem() { return statementEntry }
                    function transactionItemById(transactionId) {
                        for (let i = 0; i < transactionRepeater.count; ++i) {
                            const item = transactionRepeater.itemAt(i)
                            if (item && item.transactionId === transactionId) return item
                        }
                        return null
                    }

                    Column {
                        id: statementContent
                        anchors.fill: parent
                        anchors.margins: root.theme.spacingSmall
                        spacing: root.theme.spacingSmall

                        Item {
                            width: parent.width
                            height: Math.max(root.rowHeight - (root.theme.spacingSmall * 2),
                                             statementNameText.implicitHeight + (root.theme.margins * 2),
                                             collapseButton.implicitHeight + (root.theme.margins * 2))

                            MouseArea {
                                objectName: "bookingStatementMouse_" + statementEntry.statementId
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton
                                preventStealing: true
                                function triggerClick() {
                                    if (!root.session) return
                                    root.session.selectedStatementId = statementEntry.statementId
                                    root.session.selectedTransactionId = ""
                                    Qt.callLater(root.scrollSelectionToTop)
                                }
                                onClicked: triggerClick()
                            }

                            Text {
                                id: statementNameText
                                anchors.left: parent.left
                                anchors.right: collapseButton.left
                                anchors.rightMargin: root.theme.spacingSmall
                                anchors.top: parent.top
                                anchors.topMargin: root.theme.margins
                                text: statementEntry.statementName
                                color: root.theme.textPrimary
                                elide: Text.ElideRight
                            }

                            Controls.Button {
                                id: collapseButton
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: root.theme.margins
                                anchors.rightMargin: root.theme.margins
                                implicitWidth: root.theme.spacingLarge + root.theme.margins * 4
                                implicitHeight: root.theme.spacingLarge + root.theme.margins * 4
                                fillColor: "transparent"
                                textColor: root.theme.textMuted
                                text: statementEntry.collapsed ? "\u25B6" : "\u25BC"
                                onClicked: statementEntry.collapsed = !statementEntry.collapsed
                            }
                        }

                        Column {
                            width: parent.width
                            spacing: root.theme.spacingSmall
                            visible: !statementEntry.collapsed

                            Repeater {
                                id: transactionRepeater
                                model: (root.session && statementEntry.statementId.length > 0) ? root.statementTransactionRows(statementEntry.statementId) : []

                                delegate: Rectangle {
                                    id: transactionEntry
                                    objectName: "bookingTransactionRow_" + transactionEntry.modelData.id
                                    required property var modelData
                                    property string transactionId: (transactionEntry.modelData.id !== undefined && transactionEntry.modelData.id !== null) ? transactionEntry.modelData.id : ""
                                    width: parent.width
                                    height: 42
                                    radius: 6
                                    color: root.session && transactionEntry.modelData.id === root.session.selectedTransactionId ? root.theme.selectionHighlight : "transparent"
                                    border.color: root.theme.borderSoft
                                    border.width: root.theme.borderWidthThin

                                    MouseArea {
                                        objectName: "bookingTransactionMouse_" + transactionEntry.modelData.id
                                        anchors.fill: parent
                                        acceptedButtons: Qt.LeftButton
                                        preventStealing: true
                                        function triggerClick() {
                                            if (!root.session) return
                                            root.session.selectedStatementId = statementEntry.statementId
                                            root.session.selectedTransactionId = transactionEntry.modelData.id
                                            Qt.callLater(root.scrollSelectionToTop)
                                        }
                                        onClicked: triggerClick()
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
    }

    Connections {
        target: root.session
        function onSelectedStatementIdChanged() { Qt.callLater(root.scrollSelectionToTop) }
        function onSelectedTransactionIdChanged() { Qt.callLater(root.scrollSelectionToTop) }
        function onDataRevisionChanged() { Qt.callLater(root.scrollSelectionToTop) }
    }

    Connections {
        target: statementRepeater
        function onModelChanged() { Qt.callLater(root.scrollSelectionToTop) }
    }

    Component.onCompleted: Qt.callLater(root.scrollSelectionToTop)
}
