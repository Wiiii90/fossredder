/**
 * @file core/include/core/application/export/AnalysisExportService.h
 * @brief Declares the export service for analysis outputs and optional packaging.
 */

#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

namespace core::application::exporting {

/**
 * @brief Writes configured analysis outputs (CSV/XLSX/JPG/PNG) and applies package mode.
 */
class AnalysisExportService {
public:
    /**
     * @brief Exports the configured analysis items and optionally packages them into an archive.
     * @param request Export request describing output location, formats, and source snapshot.
     * @return Export result describing success or failure.
     */
    ExportResult exportAnalyses(const ExportRequest& request) const;
};

} // namespace core::application::exporting
