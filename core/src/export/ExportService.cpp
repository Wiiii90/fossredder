/**
 * @file core/src/export/ExportService.cpp
 * @brief Dispatches export requests to format-specific exporters.
 */

#include "core/export/ExportService.h"

#include "core/constants/CoreDefaults.h"
#include "core/export/AnalysisExportService.h"
#include "core/export/CsvExporter.h"
#include "core/export/XlsxExporter.h"

namespace core::exporting {

ExportResult ExportService::exportData(const ExportRequest& request) const
{
    if (!request.analysisItems.empty()) {
        return AnalysisExportService{}.exportAnalyses(request);
    }

    switch (request.format) {
    case ExportFormat::Csv:
        return CsvExporter{}.exportData(request);
    case ExportFormat::Xlsx:
        return XlsxExporter{}.exportData(request);
    }

    return ExportResult{
        ExportStatus::UnsupportedFormat,
        request.format,
        {},
        std::string(core::constants::exportFlow::kErrorUnsupportedFormat),
        std::string(core::constants::exportFlow::kMessageUnsupportedFormat)
    };
}

} // namespace core::exporting
