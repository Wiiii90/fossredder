import QtQuick 2.15
import QtQuick.Layouts 1.3
import FossRedder.Components 1.0 as Components
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    readonly property var navigation: root.appContext ? root.appContext.navigation : null
    

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.margins: root.theme.settings.margin
        spacing: root.theme.spacingSmall

        

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: [
                { id: "general", cat: 0, text: qsTr("General") },
                { id: "appearance", cat: 1, text: qsTr("Appearance") },
                { id: "import", cat: 2, text: qsTr("Import") },
                { id: "export", cat: 3, text: qsTr("Export") },
                { id: "advanced", cat: 4, text: qsTr("Advanced") }
            ]
            spacing: root.theme.settings.sidebarSpacing

            delegate: Components.ListRow {
                required property int index
                required property var modelData
                theme: root.theme
                width: list.width
                text: modelData.text
                subtitle: ""
                selected: root.navigation ? (root.navigation.settingsCategoryValue === modelData.cat) : (list.currentIndex === index)
                onActivated: {
                    list.currentIndex = index
                    if (root.navigation) root.navigation.setSettingsCategoryValue(modelData.cat)
                }
            }
        }

    Component.onCompleted: {
        if (root.navigation) {
            for (let i = 0; i < list.count; ++i) {
                if (list.model[i] && list.model[i].cat === root.navigation.settingsCategoryValue) { list.currentIndex = i; return }
            }
            list.currentIndex = 0
        } else list.currentIndex = 0
    }
    }
}

