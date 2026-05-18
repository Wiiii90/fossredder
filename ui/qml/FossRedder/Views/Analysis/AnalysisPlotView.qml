/**
 * @file ui/qml/FossRedder/Views/Analysis/AnalysisPlotView.qml
 * @brief Shows the Core/Infra-rendered analysis preview image.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
pragma ComponentBehavior: Bound

Item {
    id: root
    required property var appContext
    required property var theme
    property string exportStateJson: "{}"
    signal exportStateChanged(string exportStateJson)

    readonly property var session: root.appContext ? root.appContext.session : null
    readonly property string renderedPreviewSource: root.previewImageSource()
    readonly property bool hasRenderedPreview: root.renderedPreviewSource.length > 0

    Accessible.ignored: root.appContext ? root.appContext.isDebugBuild : false

    function previewImageSource() {
        const result = root.session ? root.session.lastAnalysisResult : null
        if (!result || !result.artifacts || result.artifacts.length === undefined || result.artifacts.length === 0)
            return ""

        const rawPath = String(result.artifacts[0] || "").trim()
        if (rawPath.length === 0)
            return ""
        if (rawPath.indexOf("file:") === 0 || rawPath.indexOf("qrc:") === 0 || rawPath.indexOf("http:") === 0 || rawPath.indexOf("https:") === 0 || rawPath.indexOf("data:") === 0)
            return rawPath

        return "file:///" + rawPath.replace(/\\/g, "/")
    }

    function chartPlotMinimumHeight() {
        return root.theme && typeof root.theme.chartPlotMinimumHeight === "number" ? root.theme.chartPlotMinimumHeight : 180
    }

    function chartPlotPreferredHeight() {
        return root.theme && typeof root.theme.chartPlotPreferredHeight === "number" ? root.theme.chartPlotPreferredHeight : 260
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.theme && typeof root.theme.spacingSmall === "number" ? root.theme.spacingSmall : 6

        Image {
            id: renderedPreviewImage
            objectName: "analysisPreviewImage"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.chartPlotMinimumHeight()
            Layout.preferredHeight: root.chartPlotPreferredHeight()
            visible: root.hasRenderedPreview
            source: root.renderedPreviewSource
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
            verticalAlignment: Image.AlignTop
            smooth: true
            mipmap: true
            asynchronous: false
            cache: false
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.chartPlotMinimumHeight()
            visible: !root.hasRenderedPreview
            color: root.theme && root.theme.surfaceAlt ? root.theme.surfaceAlt : "#f5f5f5"
            border.width: root.theme && typeof root.theme.borderWidthThin === "number" ? root.theme.borderWidthThin : 1
            border.color: root.theme && root.theme.borderLight ? root.theme.borderLight : "#d7d7d7"

            Label {
                anchors.centerIn: parent
                text: qsTr("No plot preview available")
                color: root.theme && root.theme.textMuted ? root.theme.textMuted : "#666666"
            }
        }
    }
}
