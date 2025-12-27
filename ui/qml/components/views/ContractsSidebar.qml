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
                text: qsTr("Contracts")
                font.pointSize: 14
            }

            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: uiData ? uiData.contracts : null

                delegate: Rectangle {
                    width: list.width
                    height: 36
                    color: (uiData && model.id === uiData.selectedContractId) ? "#ffd39c" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        Label { text: model.name; Layout.fillWidth: true }
                        Label { text: model.type; opacity: 0.7 }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (uiData) uiData.selectedContractId = model.id
                        }
                    }
                }
            }
        }
    }
}
