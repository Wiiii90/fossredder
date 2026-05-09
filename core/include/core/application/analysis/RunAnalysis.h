#pragma once

#include "core/application/analysis/RunAnalysisRequest.h"
#include "core/application/analysis/RunAnalysisResult.h"

#include <memory>
#include <string>

namespace core::domain {
struct WorkspaceState;
class Analysis;
}

namespace core::application::analysis {

class RunAnalysis {
public:
    RunAnalysis();
    ~RunAnalysis();

    RunAnalysis(const RunAnalysis&) = delete;
    RunAnalysis& operator=(const RunAnalysis&) = delete;
    RunAnalysis(RunAnalysis&&) noexcept;
    RunAnalysis& operator=(RunAnalysis&&) noexcept;

    RunAnalysisResult runAnalysis(const core::domain::WorkspaceState& state, const RunAnalysisRequest& request) const;
    RunAnalysisResult runAnalysisById(const core::domain::WorkspaceState& state,
                                      const std::string& analysisId,
                                      const std::string& filterSpec = {}) const;

    RunAnalysisResult computeAnalysisById(const core::domain::WorkspaceState& state,
                                          const std::string& analysisId,
                                          const std::string& filterSpec = {}) const;
    RunAnalysisResult computeAnalysis(const core::domain::Analysis& analysis,
                                      const core::domain::WorkspaceState& state,
                                      const std::string& filterSpec = {}) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core::application::analysis

namespace core::application {
using AnalysisService = analysis::RunAnalysis;
}
