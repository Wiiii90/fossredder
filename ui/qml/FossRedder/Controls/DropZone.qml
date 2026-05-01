/**
 * @file P:/fossredder-ui/ui/qml/FossRedder/Controls/DropZone.qml
 * @brief Provides the DropZone component.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0 as FR
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    readonly property var theme: FR.Theme
    implicitHeight: Math.max(dropZoneMinimumHeight, dropZoneRect.implicitHeight)
    property int dropZoneMinimumHeight: 140

    property string title: qsTr("Drop PDFs here")
    property string subtitle: qsTr("")

    property bool allowBrowse: true
    property string browseText: qsTr("Browse...")
    property bool clickToBrowse: true

    property var files: []
    property int queuedCount: 0

    signal browseRequested()

    function fileName(path) {
        if (!path) return ""
        const s = String(path).replace(/\\/g, "/")
        const idx = s.lastIndexOf("/")
        return idx >= 0 ? s.substring(idx + 1) : s
    }

    function fileSummary() {
        if (!root.files || root.files.length === 0) return ""
        const names = []
        for (let i = 0; i < root.files.length; ++i) {
            const n = root.fileName(root.files[i])
            if (n) names.push(n)
        }
        return names.length > 0 ? qsTr("Selected: %1").arg(names.join(", ")) : ""
    }

    Rectangle {
        id: dropZoneRect
        anchors.fill: parent
        radius: root.theme.radius
        color: root.theme.surfaceAlt
        border.width: 1
        border.color: root.theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.theme.spacing
            spacing: root.theme.spacingSmall

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "../Assets/import.svg"
                width: root.theme.viewSectionIconSize
                height: root.theme.viewSectionIconSize
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.85
            }

            Label {
                Layout.fillWidth: true
                text: root.title
                font.bold: true
                color: root.theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: root.subtitle
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: root.subtitle && root.subtitle.length > 0
            }

            Label {
                Layout.fillWidth: true
                visible: root.files && root.files.length > 0
                text: root.fileSummary()
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                visible: root.queuedCount > 0
                text: qsTr("Queue: %1").arg(root.queuedCount)
                color: root.theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            Controls.SecondaryButton {
                visible: root.allowBrowse
                Layout.alignment: Qt.AlignHCenter
                text: root.browseText
                onClicked: root.browseRequested()
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.clickToBrowse && root.enabled
        cursorShape: Qt.PointingHandCursor
        onClicked: root.browseRequested()
    }
}
