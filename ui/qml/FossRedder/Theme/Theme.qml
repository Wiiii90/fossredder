import QtQuick 2.15
pragma Singleton

QtObject {
    property color primary: "#1976D2"
    property color accent: "#03A9F4"
    property color background: "#fff8e1"
    property color surface: "#FFFFFF"
    property color surfaceAlt: "#F6F7F9"
    property color border: "#D6DADF"
    property color textPrimary: "#213547"
    property color textMuted: "#6f7d89"

    property color danger: "#C62828"
    property color success: "#2E7D32"
    property color warning: "#ff9800"
    property color info: "#2196f3"
    property color neutral: "#9e9e9e"
    property color successStrong: "#4caf50"
    property color dangerStrong: "#e53935"
    property color borderSoft: "#e6e6e6"
    property color borderLight: "#eeeeee"
    property color borderMedium: "#dddddd"
    property color borderStrong: "#cccccc"
    property color statusBarBackground: "#f6fbfd"
    property color statusBarBorder: "#e3eef6"
    property color placeholderText: "#666666"
    property color debugText: "#444444"
    property color selectionHighlight: "#ffd39c"
    property color toolbarBackground: "#f6fbfd"
    property color toolbarBorder: "#e6eef5"
    property color divider: "#d0e6f1"
    property color chartText: "#333333"
    property color chartFallback: "#888888"
    property color shadow: "#000000"

    property int margins: 2
    property int spacing: 12
    property int spacingMedium: 8
    property int spacingSmall: 6
    property int spacingLarge: 20
    property int pageMargin: 12
    property int shellMinimumHeight: 100
    property int shellSidebarMinimumWidth: 100
    property int shellSidebarPreferredWidth: 320
    property int shellContentMinimumWidth: 200
    property int toolbarHeight: 80
    property int toolbarIconButtonWidth: 72
    property int toolbarGroupSpacing: 12
    property int toolbarSectionSpacing: 6
    property int statusBarHeight: 36
    property int formLabelWidth: 120
    property int formFieldWidth: 200
    property int busyIndicatorSize: 24
    property int chartPanelMargin: 6
    property int chartPanelSpacing: 6
    property int chartPlotPreferredHeight: 320
    property int chartPlotMinimumHeight: 200
    property int chartLegendHeight: 120
    property int chartLegendMarkerSize: 12
    property int chartValueLabelWidth: 80
    property int chartPercentLabelWidth: 60
    property int chartPropertyChipHeight: 22
    property int chartPropertyChipMinWidth: 80
    property int radius: 3
    property int borderWidthThin: 1

    property var settings: ({
        margin: 8,
        spacing: 8,
        panelPreferredWidth: 760,
        sidebarSpacing: 6
    })

    property var analysis: ({
        palette: ["#8dd3c7", "#ffffb3", "#bebada", "#fb8072", "#80b1d3", "#fdb462", "#b3de69", "#fccde5", "#d9d9d9", "#bc80bd", "#ccebc5", "#ffed6f"],
        layout: {
            defaultWidth: 800,
            defaultHeight: 600,
            splitControlsWidth: 220,
            splitAnimationDurationMs: 400,
            initialPaintIntervalMs: 300,
            initialPaintReadyWidth: 120,
            initialPaintMaxAttempts: 6,
            repaintDelayMs: 100,
            minDebugRepaintWidth: 50,
            minRenderWidth: 100,
            legendTopMargin: 8
        },
        render: {
            pieStartAngle: -Math.PI / 2,
            pieRadiusPadding: 10,
            compactBarLeftPadding: 20,
            compactBarVerticalSpacing: 18,
            compactBarTopOffset: 4,
            compactBarHeight: 12,
            compactBarWidthPadding: 40,
            compactBarMinWidth: 2,
            histogramGroupPadding: 8,
            histogramBottomPadding: 18,
            histogramTopPadding: 30,
            propertyBarInset: 2,
            propertyLabelMinWidth: 36
        },
        table: {
            rowHeight: 28,
            dateColumnWidth: 140,
            amountColumnWidth: 100,
            contractColumnWidth: 160,
            detailListHeight: 300
        },
        calc: {
            defaultWidth: 400,
            taxFieldWidth: 80,
            transactionListHeight: 220,
            nameColumnWidth: 200
        }
    })

    property string fontFamily: "Segoe UI"

    property int fontSizeSmall: 11
    property int fontSize: 12
    property int fontSizeLarge: 14
    property int fontSizeTitle: 16
}
