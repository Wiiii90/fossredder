/**
 * @file core/include/core/application/export/ExportService.h
 * @brief Declares the export entry point used by the application shell.
 */

#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

namespace core::application::exporting {

/**
 * @brief Dispatches export requests to the format-specific exporters.
 */
class ExportService {
public:
    /**
     * @brief Dispatches an export request to the appropriate exporter implementation.
     * @param request Export request describing the desired output.
     * @return Export result describing success or failure.
     */
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::application::exporting
