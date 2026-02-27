import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import FossRedder 1.0
import FossRedder.Controls 1.0 as Controls

Item {
    id: root
    implicitHeight: 140

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
        var s = String(path).replace(/\\/g, "/")
        var idx = s.lastIndexOf("/")
        return idx >= 0 ? s.substring(idx + 1) : s
    }

    function fileSummary() {
        if (!root.files || root.files.length === 0) return ""
        var names = []
        for (var i = 0; i < root.files.length; ++i) {
            var n = fileName(root.files[i])
            if (n) names.push(n)
        }
        return names.length > 0 ? qsTr("Selected: %1").arg(names.join(", ")) : ""
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radius
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacing
            spacing: Theme.spacingSmall

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/qml/FossRedder/Assets/import.svg"
                width: 28
                height: 28
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.85
            }

            Label {
                Layout.fillWidth: true
                text: root.title
                font.bold: true
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: root.subtitle
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: text && text.length > 0
            }

            Label {
                Layout.fillWidth: true
                visible: root.files && root.files.length > 0
                text: root.fileSummary()
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.fillWidth: true
                visible: root.queuedCount > 0
                text: qsTr("Queue: %1").arg(root.queuedCount)
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            Controls.Button {
                visible: root.allowBrowse
                Layout.alignment: Qt.AlignHCenter
                text: root.browseText
                fillColor: Theme.surface
                textColor: Theme.textPrimary
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
