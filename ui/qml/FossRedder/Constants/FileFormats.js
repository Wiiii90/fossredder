.pragma library

var importSources = {
    pdf: {
        extension: "pdf",
        label: "PDF"
    }
}

var exportFormats = {
    csv: {
        extension: "csv",
        label: "CSV"
    },
    xlsx: {
        extension: "xlsx",
        label: "XLSX"
    }
}

var exportDefaults = {
    baseName: "export"
}

function dotExtension(extension) {
    return "." + extension
}

function supportedExportLabels() {
    return [exportFormats.csv.label, exportFormats.xlsx.label]
}
