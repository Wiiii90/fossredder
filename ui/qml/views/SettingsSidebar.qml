import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root
    signal categoryRequested(string src)

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.margins: 8
        spacing: Theme.spacingSmall

        // Sidebar header removed to avoid duplicate top-level title; Content wrapper will show the main "Settings" header

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: [
                { id: "general", cat: 0, text: qsTr("General") },
                { id: "appearance", cat: 1, text: qsTr("Appearance") },
                { id: "import", cat: 2, text: qsTr("Import") },
                { id: "export", cat: 3, text: qsTr("Export") },
                { id: "storage", cat: 4, text: qsTr("Storage & Privacy") },
                { id: "notifications", cat: 5, text: qsTr("Notifications") },
                { id: "advanced", cat: 6, text: qsTr("Advanced") },
                { id: "updates", cat: 7, text: qsTr("Updates") }
            ]
            spacing: 6

            delegate: Rectangle {
                width: list.width
                height: 44
                radius: Theme.radius
                color: (list.currentIndex === index) ? Theme.background : "transparent"
                border.color: "#e6e6e6"
                border.width: 1

                Label { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 8; text: modelData.text; color: Theme.textPrimary }

                MouseArea { anchors.fill: parent; onClicked: {
                    list.currentIndex = index
                    try {
                        if (typeof uiNav !== 'undefined' && uiNav) {
                            uiNav.settingsCategory = modelData.cat
                        }
                    } catch(e) { }
                } }
            }
        }

    Component.onCompleted: {
        list.currentIndex = 0
    }
    }
}
