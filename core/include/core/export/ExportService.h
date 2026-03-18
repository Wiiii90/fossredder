/**
 * @file core/include/core/export/ExportService.h
 * @brief Declares the export entry point used by the application shell.
 */

#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::exporting {

/**
 * @brief Dispatches export requests to the format-specific exporters.
 */
class ExportService {
public:
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::exporting
