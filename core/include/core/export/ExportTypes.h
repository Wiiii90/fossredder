/**
 * @file core/include/core/export/ExportTypes.h
 * @brief Shared export enums used by request, result, and controller entry points.
 */

#pragma once

namespace core::controllers::exporting {

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

} // namespace core::controllers::exporting
