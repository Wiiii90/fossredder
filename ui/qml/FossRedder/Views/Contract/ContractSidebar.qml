import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingSmall

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contractColumn.implicitHeight

            Column {
                id: contractColumn
                width: parent.width
                spacing: Theme.spacingSmall

                Repeater {
                    model: uiData ? uiData.contractRows() : []

                    delegate: Rectangle {
                        width: contractColumn.width
                        height: 44
                        radius: 6
                        color: uiData && modelData.id === uiData.selectedContractId ? Theme.selectionHighlight : "transparent"
                        border.color: Theme.borderSoft
                        border.width: Theme.borderWidthThin

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (uiData) uiData.selectedContractId = modelData.id
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: Theme.spacingSmall
                            spacing: 2

                            Text {
                                width: parent.width
                                text: modelData.name ? modelData.name : ""
                                color: Theme.textPrimary
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}

