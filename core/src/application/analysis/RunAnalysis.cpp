/**
 * @file core/src/application/analysis/RunAnalysis.cpp
 * @brief Implements application-level analysis execution services.
 */

#include "core/application/analysis/RunAnalysis.h"

#include "core/application/analysis/AnalysisEngine.h"
#include "core/application/workspace/AppState.h"

#include <utility>

namespace core::application::analysis {

class RunAnalysis::Impl {
public:
    core::analysis::AnalysisEngine engine;
};

RunAnalysis::RunAnalysis()
    : impl_(std::make_unique<Impl>())
{
}

RunAnalysis::~RunAnalysis() = default;

RunAnalysis::RunAnalysis(RunAnalysis&&) noexcept = default;

RunAnalysis& RunAnalysis::operator=(RunAnalysis&&) noexcept = default;

RunAnalysisResult RunAnalysis::runAnalysis(const core::domain::AppState& state,
                                           const RunAnalysisRequest& request) const
{
    return runAnalysisById(state, request.analysisId, request.filterSpec);
}

RunAnalysisResult RunAnalysis::runAnalysisById(const core::domain::AppState& state,
                                               const std::string& analysisId,
                                               const std::string& filterSpec) const
{
    return impl_->engine.computeAnalysisById(analysisId, state, filterSpec);
}

RunAnalysisResult RunAnalysis::computeAnalysisById(const core::domain::AppState& state,
                                                   const std::string& analysisId,
                                                   const std::string& filterSpec) const
{
    return runAnalysisById(state, analysisId, filterSpec);
}

RunAnalysisResult RunAnalysis::computeAnalysis(const core::domain::Analysis& analysis,
                                               const core::domain::AppState& state,
                                               const std::string& filterSpec) const
{
    return impl_->engine.computeAnalysis(analysis, state, filterSpec);
}

} // namespace core::application::analysis
