/**
 * @file core/include/core/application/AnalysisService.h
 * @brief Declares application-level analysis execution services.
 */

#pragma once

#include "core/models/AnalysisResult.h"

#include <memory>
#include <string>

namespace core::domain {
struct AppState;
}

namespace core::application {

class AnalysisService {
public:
    AnalysisService();
    ~AnalysisService();

    AnalysisService(const AnalysisService&) = delete;
    AnalysisService& operator=(const AnalysisService&) = delete;
    AnalysisService(AnalysisService&&) noexcept;
    AnalysisService& operator=(AnalysisService&&) noexcept;

    core::domain::AnalysisResult computeAnalysisById(const core::domain::AppState& state,
                                       const std::string& analysisId,
                                       const std::string& filterSpec = {}) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
