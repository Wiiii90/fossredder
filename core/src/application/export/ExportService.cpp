/**
 * @file core/src/export/ExportService.cpp
 * @brief Dispatches export requests to format-specific exporters.
 */

#include "core/application/export/ExportService.h"

#include "core/application/export/CsvExporter.h"
#include "core/application/export/XlsxExporter.h"
#include "core/constants/export.h"
#include "ObjectExportExecutor.h"

#include <set>
#include <unordered_map>

namespace core::application::exporting {

namespace {

std::string dedupeKey(const ExportObjectRequest& item)
{
    return item.annualId + "|" + item.objectId + "|" + std::to_string(static_cast<int>(item.format));
}

AnalysisExportFormat analysisFormatFromString(const std::string& value)
{
    if (value == "xlsx") return AnalysisExportFormat::Xlsx;
    if (value == "jpg" || value == "jpeg") return AnalysisExportFormat::Jpg;
    if (value == "png") return AnalysisExportFormat::Png;
    return AnalysisExportFormat::Csv;
}

std::vector<ExportObjectRequest> expandObjectRequests(const ExportRequest& request)
{
    std::vector<ExportObjectRequest> out;
    if (!request.stateSnapshot) {
        return out;
    }

    std::unordered_map<std::string, std::shared_ptr<core::domain::Analysis>> analysisById;
    for (const auto& analysis : request.stateSnapshot->analyses()) {
        if (!analysis || analysis->id().empty()) continue;
        analysisById.emplace(analysis->id(), analysis);
    }

    std::set<std::string> seen;
    for (const auto& item : request.objectRequests) {
        if (item.type == ExportObjectType::Analysis) {
            if (!item.objectId.empty()) {
                const std::string key = dedupeKey(item);
                if (seen.find(key) != seen.end()) continue;
                seen.insert(key);
                out.push_back(item);
            }
            continue;
        }

        for (const auto& annual : request.stateSnapshot->annuals()) {
            if (!annual || annual->id() != item.objectId) continue;
            for (const auto& analysisId : annual->analysisIds()) {
                const auto it = analysisById.find(analysisId);
                if (it == analysisById.end() || !it->second) continue;
                ExportObjectRequest expanded;
                expanded.type = ExportObjectType::Analysis;
                expanded.objectId = analysisId;
                expanded.annualId = annual->id();
                expanded.name = it->second->name();
                expanded.format = analysisFormatFromString(it->second->exportFormat());
                const std::string key = dedupeKey(expanded);
                if (seen.find(key) != seen.end()) continue;
                seen.insert(key);
                out.push_back(std::move(expanded));
            }
            break;
        }
    }

    return out;
}

} // namespace

ExportService::ExportService(std::shared_ptr<core::ports::archive::IArchive> archive,
                             std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter,
                             std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer)
    : archive_(std::move(archive)),
      xlsxWriter_(std::move(xlsxWriter)),
      imageRenderer_(std::move(imageRenderer)) {
}

ExportResult ExportService::exportData(const ExportRequest& request) const
{
    if (!request.objectRequests.empty()) {
        auto normalized = request;
        normalized.objectRequests = expandObjectRequests(request);
        if (normalized.progressCallback) {
            normalized.progressCallback(0.05, "Preparing export");
            normalized.progressCallback(0.20, "Resolving export objects");
        }
        auto result = exportObjectRequests(normalized, archive_, xlsxWriter_, imageRenderer_);
        if (normalized.progressCallback) {
            normalized.progressCallback(result.success ? 1.0 : 0.95, result.success ? "Finished" : "Failed");
        }
        return result;
    }

    switch (request.format) {
    case ExportFormat::Csv:
        return CsvExporter{}.exportData(request);
    case ExportFormat::Xlsx:
        return XlsxExporter{xlsxWriter_}.exportData(request);
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
