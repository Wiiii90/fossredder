/**
 * @file core/src/analysis/OutputTypeResolver.cpp
 * @brief Implements private output-type resolution helpers for the analysis engine.
 */

#include "OutputTypeResolver.h"

#include "core/constants/analysis.h"
#include "core/constants/json.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/domain/entities/Analysis.h"

#include <nlohmann/json.hpp>

namespace core::analysis {

std::string resolveAnalysisOutputType(const Analysis& analysis)
{
    if (analysis.type != core::constants::analysis::kTypePlot) return analysis.type;

    if (analysis.configJson.empty()) {
        return std::string(core::constants::analysis::plotTypes::kPie);
    }

    try {
        const auto config = nlohmann::json::parse(analysis.configJson);
        if (config.contains(core::constants::analysis::kPlotTypeKey) && config[core::constants::analysis::kPlotTypeKey].is_string()) {
            return config[core::constants::analysis::kPlotTypeKey].get<std::string>();
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::ExceptionError,
                                      "core::analysis::resolveAnalysisOutputType",
                                      std::current_exception());
    }

    return std::string(core::constants::analysis::plotTypes::kPie);
}

}
