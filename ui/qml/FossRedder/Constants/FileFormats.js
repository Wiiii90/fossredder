.pragma library

var importSources = {
    pdf: {
        extension: "pdf",
        label: "PDF"
    }
}

var importStrategies = {
    statement: {
        commerzbank26: {
            key: "commerzbank26",
            label: "Commerzbank26"
        }
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

function supportedImportSourceLabels() {
    return [importSources.pdf.label]
}

function supportedStatementStrategyLabels() {
    return [importStrategies.statement.commerzbank26.label]
}

function isSupportedImportPath(path) {
    if (!path)
        return false

    return String(path).toLowerCase().endsWith(dotExtension(importSources.pdf.extension))
}
