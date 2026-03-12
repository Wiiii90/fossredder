/**
 * @file core/include/core/export/ExportRequest.h
 * @brief Declares the immutable export request contract.
 */

#pragma once

#include "core/export/ExportTypes.h"
#include "core/models/AppState.h"

#include <memory>
#include <string>

namespace core::controllers::exporting {

struct ExportRequest {
    ExportFormat format = ExportFormat::Xlsx;
    std::string outputPath;
    bool includeFormulas = true;
    std::string locale;
    std::shared_ptr<const core::domain::AppState> stateSnapshot;
};

} // namespace core::controllers::exporting
