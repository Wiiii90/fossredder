/**
 * @file core/include/core/export/AnalysisExportService.h
 * @brief Declares export of analysis outputs to table/image files and optional ZIP archive creation.
 */

#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::exporting {

/**
 * @brief Writes configured analysis outputs (CSV/XLSX/JPG/PNG) and applies package mode.
 */
class AnalysisExportService {
public:
    ExportResult exportAnalyses(const ExportRequest& request) const;
};

} // namespace core::exporting
