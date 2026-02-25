import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import "qrc:/qml/components/controls"

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: Theme.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Gebäude"); font.pointSize: 14; color: Theme.textPrimary; Layout.fillWidth: true }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: uiData ? uiData.properties : null
            spacing: 6

            delegate: Rectangle {
                width: list.width
                height: 48
                radius: Theme.radius
                color: (uiData && model.id === uiData.selectedPropertyId) ? Theme.background : "transparent"
                border.color: "#e6e6e6"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Label { text: model.name; color: Theme.textPrimary }
                    Label { text: model.address; color: Theme.textMuted; font.pointSize: Math.max(12, Theme.fontSize - 2) }
                }

                MouseArea { anchors.fill: parent; onClicked: if (uiData) uiData.selectedPropertyId = model.id }
            }
        }
    }
}
