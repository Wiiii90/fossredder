/**
 * @file ui/qml/FossRedder/Views/Contract/ContractView.qml
 * @brief Provides the ContractView component.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Views.Contract 1.0 as Contract
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var workspaceFacade: root.appContext ? root.appContext.workspaceFacade : null
    readonly property var sessionState: root.workspaceFacade ? root.workspaceFacade.session : null
    readonly property var contractState: root.workspaceFacade ? root.workspaceFacade.contractState : null
    readonly property var contractRows: root.workspaceFacade ? root.workspaceFacade.contractRows : []
    readonly property bool isEdit: root.contractState ? root.contractState.isEdit : false
    readonly property bool hasChanges: root.contractState ? root.contractState.hasChanges : false
    readonly property string name: root.contractState ? root.contractState.name : ""
    readonly property string type: root.contractState ? root.contractState.type : ""
    readonly property var aliases: root.contractState ? root.contractState.aliases : []
    readonly property string aliasInputText: root.contractState ? root.contractState.aliasInputText : ""
    readonly property int aliasIndex: root.contractState ? root.contractState.aliasIndex : -1
    readonly property var selectedActorIds: root.contractState ? root.contractState.selectedActorIds : []
    readonly property var selectedPropertyIds: root.contractState ? root.contractState.selectedPropertyIds : []

    function syncContractStateFromSelection(forceReload) {
        if (root.contractState && root.contractState["syncFromSelection"])
            root.contractState["syncFromSelection"](forceReload)
    }

    Connections {
        target: root.sessionState
        ignoreUnknownSignals: true

        function onSelectedContractChanged() { root.syncContractStateFromSelection(true) }
        function onSelectedContractIdChanged() { root.syncContractStateFromSelection(true) }
        function onDataRevisionChanged() { root.syncContractStateFromSelection(true) }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme.spacingSmall

        Contract.ContractForm {
            Layout.fillWidth: true
            Layout.fillHeight: true
            appContext: root.appContext
            contractState: root.contractState
            theme: root.theme
        }

        Contract.ContractBottomBar {
            Layout.fillWidth: true
            Layout.leftMargin: root.theme.pageContentMargin
            Layout.rightMargin: root.theme.pageContentMargin
            Layout.bottomMargin: root.theme.pageContentMargin
            theme: root.theme
            contractState: root.contractState
            contractRows: root.contractRows
        }
    }
}
