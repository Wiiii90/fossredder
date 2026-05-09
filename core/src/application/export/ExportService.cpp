/**
 * @file core/src/export/ExportService.cpp
 * @brief Dispatches export requests to format-specific exporters.
 */

#include "core/application/export/ExportService.h"

#include "core/application/export/AnalysisExportService.h"
#include "core/application/export/CsvExporter.h"
#include "core/application/export/XlsxExporter.h"
#include "core/constants/export.h"

namespace core::application::exporting {

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
        false,
        ExportStatus::UnsupportedFormat,
        request.format,
        {},
        std::string(core::constants::exportFlow::kErrorUnsupportedFormat),
        std::string(core::constants::exportFlow::kMessageUnsupportedFormat)
    };
}

} // namespace core::application::exporting
