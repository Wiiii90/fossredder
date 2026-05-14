/**
 * @file core/include/core/application/export/ExportRequest.h
 * @brief Declares the export request contract and per-item export descriptors.
 */

#pragma once

#include "core/application/export/ExportTypes.h"
#include "core/domain/catalog/WorkspaceCatalog.h"

#include <memory>
#include <string>
#include <vector>

namespace core::application::exporting {

/**
 * @brief Describes one analysis item to export.
 */
struct AnalysisExportItem {
    std::string annualId;
    std::string analysisId;
    AnalysisExportFormat format = AnalysisExportFormat::Csv;
    std::string name;
};

/**
 * @brief Describes one export run.
 */
struct ExportRequest {
    ExportFormat format = ExportFormat::Xlsx;
    std::string outputPath;
    bool includeFormulas = true;
    std::string locale;
    PackageFormat packageFormat = PackageFormat::None;
    std::vector<AnalysisExportItem> analysisItems;
    std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> stateSnapshot;
};

} // namespace core::application::exporting
