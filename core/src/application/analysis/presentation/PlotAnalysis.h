#pragma once

#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"

namespace core::application::analysis {
class AnalysisFilter;
}

namespace core::application::analysis {

AnalysisResult computePlotAnalysis(const core::domain::Analysis& analysis,
                                   const core::domain::catalog::WorkspaceCatalog& state,
                                   const AnalysisFilter& filter);

} // namespace core::application::analysis
