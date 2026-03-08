.pragma library

var formatIndexes = {
    csv: 0,
    xlsx: 1
}

var fileExtensions = {
    csv: "csv",
    xlsx: "xlsx"
}

var fileNames = {
    exportBaseName: "export"
}

function extensionForFormat(formatIndex) {
    return formatIndex === formatIndexes.csv ? fileExtensions.csv : fileExtensions.xlsx
}
