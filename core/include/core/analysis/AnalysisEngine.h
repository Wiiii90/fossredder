/**
 * @file core/include/core/analysis/AnalysisEngine.h
 * @brief Declares the public analysis engine entry point.
 */

#pragma once

#include <memory>
#include "core/models/AnalysisResult.h"

#include <string>

namespace core::domain {
struct AppState;
class Analysis;
}

// AnalysisEngine resolves analysis strategies and executes analyses against
// an core::domain::AppState. It is intentionally lightweight and stateless; strategies are
// owned by the engine and can be extended or replaced for testing.
namespace core::analysis {

class AnalysisEngine {
public:
    AnalysisEngine();
    ~AnalysisEngine();

    AnalysisEngine(const AnalysisEngine&) = delete;
    AnalysisEngine& operator=(const AnalysisEngine&) = delete;
    AnalysisEngine(AnalysisEngine&&) noexcept;
    AnalysisEngine& operator=(AnalysisEngine&&) noexcept;

    core::domain::AnalysisResult computeAnalysisById(const std::string& analysisId, const core::domain::AppState& state, const std::string& filterSpec = "") const;
    core::domain::AnalysisResult computeAnalysis(const core::domain::Analysis& analysis, const core::domain::AppState& state, const std::string& filterSpec = "") const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
