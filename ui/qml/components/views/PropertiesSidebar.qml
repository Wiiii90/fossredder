import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#ffe5cc"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 6

            Label {
                text: qsTr("Properties")
                font.pointSize: 14
            }

            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: uiData ? uiData.properties : null

                delegate: Rectangle {
                    width: list.width
                    height: 40
                    color: (uiData && model.id === uiData.selectedPropertyId) ? "#ffd39c" : "transparent"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 0

                        Label { text: model.name; Layout.fillWidth: true }
                        Label { text: model.address; opacity: 0.7; Layout.fillWidth: true }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (uiData) uiData.selectedPropertyId = model.id
                        }
                    }
                }
            }
        }
    }
}
