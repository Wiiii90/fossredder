.pragma library

var formatIds = {
    csv: 0,
    xlsx: 1
}

var extensions = {
    pdf: "pdf",
    csv: "csv",
    xlsx: "xlsx"
}

var fileNames = {
    exportBaseName: "export"
}

function dotExtension(extension) {
    return "." + extension
}

function extensionForExportFormat(formatId) {
    return formatId === formatIds.csv ? extensions.csv : extensions.xlsx
}
