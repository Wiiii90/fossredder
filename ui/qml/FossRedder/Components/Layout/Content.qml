import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0

Item {
    id: contentWrapper
    property string title: ""
    property string subtitle: ""
    property bool showHeader: true

    property Component contentComponent: null

    signal contentStatusChanged(int status, var source, var item)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: Theme.spacing

        Item {
            visible: showHeader && title && title.length > 0
            Layout.fillWidth: true
            ColumnLayout {
                Label { text: title; font.pointSize: 18; color: Theme.textPrimary }
                Label { text: subtitle; font.pointSize: Math.max(12, Theme.fontSize - 2); color: Theme.textMuted; visible: subtitle && subtitle.length > 0 }
                Rectangle { height: 1; color: "#eee"; Layout.fillWidth: true }
            }
        }

        Loader {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: contentWrapper.contentComponent
            onStatusChanged: {
                var hasItem = (item !== undefined && item !== null);
                contentWrapper.contentStatusChanged(status, source, hasItem ? 1 : 0);
            }
        }
    }

    function setContentComponent(c) {
        contentWrapper.contentComponent = c
    }
}
