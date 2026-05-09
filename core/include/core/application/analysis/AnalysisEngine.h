/**
 * @file core/include/core/analysis/AnalysisEngine.h
 * @brief Declares the public analysis engine entry point.
 */

#pragma once

#include <memory>
#include "core/application/analysis/RunAnalysisResult.h"

#include <string>

namespace core::domain {
struct WorkspaceState;
class Analysis;
}

namespace core::analysis {

class AnalysisEngine {
public:
    AnalysisEngine();
    ~AnalysisEngine();

    AnalysisEngine(const AnalysisEngine&) = delete;
    AnalysisEngine& operator=(const AnalysisEngine&) = delete;
    AnalysisEngine(AnalysisEngine&&) noexcept;
    AnalysisEngine& operator=(AnalysisEngine&&) noexcept;

    core::domain::AnalysisResult computeAnalysisById(const std::string& analysisId, const core::domain::WorkspaceState& state, const std::string& filterSpec = "") const;
    core::domain::AnalysisResult computeAnalysis(const core::domain::Analysis& analysis, const core::domain::WorkspaceState& state, const std::string& filterSpec = "") const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core::analysis
