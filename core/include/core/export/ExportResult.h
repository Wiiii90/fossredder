/**
 * @file core/include/core/export/ExportResult.h
 * @brief Declares the export execution result contract.
 */

#pragma once

#include "core/export/ExportTypes.h"

#include <string>

namespace core::controllers::exporting {

struct ExportResult {
    ExportStatus status = ExportStatus::InternalError;
    ExportFormat actualFormat = ExportFormat::Xlsx;
    std::string resolvedOutputPath;
    std::string errorCode;
    std::string message;
};

} // namespace core::controllers::exporting
