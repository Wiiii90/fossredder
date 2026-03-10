.pragma library

var chartTypes = {
    pie: "pie",
    histogram: "histogram"
}

var text = {
    defaultLegendValue: "0.00",
    percentSuffix: "%"
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

    if (!result.table || result.table.length === 0)
        return ""

    var sample = result.table[0]
    if (!sample || sample.length < 2)
        return chartTypes.pie

    try {
        var histogramConfig = JSON.parse(sample[1])
        if (histogramConfig && (histogramConfig.total !== undefined || histogramConfig.byContract !== undefined || histogramConfig.byProperty !== undefined))
            return chartTypes.histogram
    } catch (e) {
    }

    var numericValue = parseFloat(sample[1])
    if (!isNaN(numericValue))
        return chartTypes.pie

    if (sample.length > 2) {
        var secondaryNumericValue = parseFloat(sample[2])
        if (!isNaN(secondaryNumericValue))
            return chartTypes.pie
    }

    return chartTypes.pie
}
