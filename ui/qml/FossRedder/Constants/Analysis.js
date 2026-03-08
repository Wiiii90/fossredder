.pragma library

var chartTypes = {
    pie: "pie",
    histogram: "histogram"
}

var layout = {
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
}

var render = {
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
    propertyLabelMinWidth: 36,
    chartFontFamily: "sans-serif"
}

var text = {
    defaultLegendValue: "0.00",
    ellipsis: "...",
    percentSuffix: "%"
}

var calendar = {
    monthNames: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"]
}

function hashString(value) {
    var hash = 0
    if (!value)
        return 0

    for (var i = 0; i < value.length; ++i) {
        hash = ((hash << 5) - hash) + value.charCodeAt(i)
        hash |= 0
    }

    return Math.abs(hash)
}

function colorForKey(key, palette, fallbackColor) {
    try {
        var index = hashString(key) % palette.length
        return palette[index]
    } catch (e) {
        return fallbackColor
    }
}

function plotType(result) {
    if (!result)
        return ""
    if (result.type)
        return result.type

    if (result.config) {
        try {
            var config = JSON.parse(result.config)
            if (config && config.plotType)
                return config.plotType
        } catch (e) {
        }
    }

    return (result.table && result.table.length > 0) ? chartTypes.pie : ""
}

function formatMonthLabel(monthValue) {
    if (!monthValue)
        return monthValue

    var parts = monthValue.split("-")
    if (parts.length >= 2 && parts[0].length === 4) {
        var year = parts[0]
        var month = parts[1]
        var monthIndex = parseInt(month, 10) - 1
        if (!isNaN(monthIndex) && monthIndex >= 0 && monthIndex < calendar.monthNames.length)
            return calendar.monthNames[monthIndex] + " " + year
    }

    return monthValue
}

function shouldShowHistogramControls(result) {
    return plotType(result) === chartTypes.histogram
}

function shouldShowHistogramLegend(result, legendModel) {
    return plotType(result) === chartTypes.histogram || (legendModel && legendModel.length > 0)
}

function shouldShowNonHistogramLegend(result) {
    return plotType(result) !== chartTypes.histogram
}
