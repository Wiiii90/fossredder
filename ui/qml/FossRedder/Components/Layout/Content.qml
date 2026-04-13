import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
    id: contentWrapper
    required property var theme
    property string title: ""
    property string subtitle: ""
    property bool showHeader: true

    property Component contentComponent: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: contentWrapper.theme.pageMargin
        spacing: contentWrapper.theme.spacing

        Item {
            visible: contentWrapper.showHeader && contentWrapper.title && contentWrapper.title.length > 0
            Layout.fillWidth: true
            ColumnLayout {
                Label { text: contentWrapper.title; font.pointSize: contentWrapper.theme.fontSizeTitle + contentWrapper.theme.margins; color: contentWrapper.theme.textPrimary }
                Label { text: contentWrapper.subtitle; font.pointSize: Math.max(12, contentWrapper.theme.fontSize - 2); color: contentWrapper.theme.textMuted; visible: contentWrapper.subtitle && contentWrapper.subtitle.length > 0 }
                Rectangle { height: contentWrapper.theme.borderWidthThin; color: contentWrapper.theme.borderLight; Layout.fillWidth: true }
            }
        }

        Loader {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: contentWrapper.contentComponent
        }
    }
}
