import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: sidebar

    signal contentStatusChanged(int status, var source, var item)

    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing
        anchors.rightMargin: Theme.spacing
        anchors.bottomMargin: Theme.spacing
        anchors.topMargin: headerLabel.text && headerLabel.text.length > 0 ? Theme.spacing : 0
        spacing: Theme.spacing

        Label {
            id: headerLabel
            text: ""
            font.pointSize: 16
            color: Theme.textPrimary
            visible: headerLabel.text && headerLabel.text.length > 0
        }

        Rectangle {
            id: headerSep
            height: 1
            color: "#eee"
            Layout.fillWidth: true
            visible: headerLabel.visible
        }

        Loader {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true

            onStatusChanged: {
                var hasItem = (item !== undefined && item !== null);
                sidebar.contentStatusChanged(status, source, hasItem ? 1 : 0);
            }
        }
    }

    function setContentSource(s) {
        if (!content) return
        content.source = s
    }
}
