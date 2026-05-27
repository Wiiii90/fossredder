/**
 * @file ui/qml/FossRedder/Views/Actor/ActorView.qml
 * @brief Provides the ActorView component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views 1.0 as Views
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property var actorState: root.workspaceFacade ? root.workspaceFacade.actorState : null
    readonly property var actorRows: root.workspaceFacade ? root.workspaceFacade.actorRows : []
    readonly property var contractRows: root.workspaceFacade ? root.workspaceFacade.contractRows : []
    readonly property bool isEdit: root.actorState ? root.actorState.isEdit : false
    readonly property bool hasChanges: root.actorState ? root.actorState.hasChanges : false
    readonly property string name: root.actorState ? root.actorState.name : ""
    readonly property var aliases: root.actorState ? root.actorState.aliases : []
    readonly property string aliasInputText: root.actorState ? root.actorState.aliasInputText : ""
    readonly property int aliasIndex: root.actorState ? root.actorState.aliasIndex : -1
    readonly property var selectedContractIds: root.actorState ? root.actorState.selectedContractIds : []

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.viewFormSpacing

        Flickable {
            id: actorScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: actorContent.implicitHeight + (root.theme.pageContentMargin * 2)
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: actorContent
                x: root.theme.pageContentMargin
                y: root.theme.pageContentMargin
                width: actorScroll.width - (root.theme.pageContentMargin * 2)
                spacing: root.theme.viewFormSpacing

                Views.ActorForm {
                    Layout.fillWidth: true
                    actorState: root.actorState
                    theme: root.theme
                }

                Views.ActorContractPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: root.theme.viewSelectionPanelMinHeight
                    theme: root.theme
                    actorState: root.actorState
                    contractRows: root.contractRows
                }
            }
        }

        Views.ActorBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            actorState: root.actorState
            actorRows: root.actorRows
        }
    }
}
