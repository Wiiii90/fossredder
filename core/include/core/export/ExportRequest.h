/**
 * @file core/include/core/export/ExportRequest.h
 * @brief Declares the immutable export request contract.
 */

#pragma once

#include "core/export/ExportTypes.h"
#include "core/models/AppState.h"

#include <memory>
#include <string>
#include <vector>

namespace core::exporting {

struct AnalysisExportItem {
    std::string annualId;
    std::string analysisId;
    AnalysisExportFormat format = AnalysisExportFormat::Csv;
    std::string name;
};

struct ExportRequest {
    ExportFormat format = ExportFormat::Xlsx;
    std::string outputPath;
    bool includeFormulas = true;
    std::string locale;
    PackageFormat packageFormat = PackageFormat::None;
    std::vector<AnalysisExportItem> analysisItems;
    std::shared_ptr<const core::domain::AppState> stateSnapshot;
};

} // namespace core::exporting
