/**
 * @file core/src/analysis/OutputTypeResolver.h
 * @brief Declares private output-type resolution helpers for the analysis engine.
 */

#pragma once

#include <string>

namespace core::domain {
class Analysis;
}

namespace core::analysis {

std::string resolveAnalysisOutputType(const core::domain::Analysis& analysis);

}
