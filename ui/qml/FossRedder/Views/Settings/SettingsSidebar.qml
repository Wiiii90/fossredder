import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Components 1.0 as Components

Item {
    id: root
    

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.margins: Theme.settings.margin
        spacing: Theme.spacingSmall

        

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
            spacing: Theme.settings.sidebarSpacing

            delegate: Components.ListRow {
                width: list.width
                text: modelData.text
                subtitle: ""
                selected: uiNav ? (uiNav.settingsCategory === modelData.cat) : (list.currentIndex === index)
                onActivated: {
                    list.currentIndex = index
                    if (uiNav) uiNav.settingsCategory = modelData.cat
                }
            }
        }

    Component.onCompleted: {
        if (uiNav) {
            for (var i = 0; i < list.count; ++i) {
                if (list.model[i] && list.model[i].cat === uiNav.settingsCategory) { list.currentIndex = i; return }
            }
            list.currentIndex = 0
        } else list.currentIndex = 0
    }
    }
}

