/**
 * @file core/include/core/application/export/ExportTypes.h
 * @brief Declares the export enumerations used by the application export layer.
 */

#pragma once

namespace core::application::exporting {

enum class ExportFormat {
    Csv,
    Xlsx
};

enum class AnalysisExportFormat {
    Csv,
    Xlsx,
    Jpg,
    Png
};

enum class PackageFormat {
    None,
    Zip
};

enum class ExportStatus {
    InternalError,
    UnsupportedFormat,
    InvalidInput,
    WriteFailed,
    XlsxGenerationFailed,
    ArchiveFailed,
    Ok
};

} // namespace core::application::exporting
