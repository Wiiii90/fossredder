/**
 * @file core/include/core/export/ExportTypes.h
 * @brief Shared export enums used by request, result, and export service entry points.
 */

#pragma once

namespace core::exporting {

enum class ExportFormat {
    Csv,
    Xlsx
};

enum class ExportStatus {
    Ok,
    InvalidInput,
    WriteFailed,
    XlsxGenerationFailed,
    UnsupportedFormat,
    InternalError
};

} // namespace core::exporting
