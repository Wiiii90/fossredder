/**
 * @file core/src/application/AnalysisService.cpp
 * @brief Implements application-level analysis execution services.
 */

#include "core/application/AnalysisService.h"

#include "core/analysis/AnalysisEngine.h"
#include "core/models/AppState.h"

namespace core::application {

class AnalysisService::Impl {
public:
    core::analysis::AnalysisEngine engine;
};

AnalysisService::AnalysisService()
    : impl_(std::make_unique<Impl>())
{
}

AnalysisService::~AnalysisService() = default;

AnalysisService::AnalysisService(AnalysisService&&) noexcept = default;

AnalysisService& AnalysisService::operator=(AnalysisService&&) noexcept = default;

AnalysisResult AnalysisService::computeAnalysisById(const AppState& state,
                                                    const std::string& analysisId,
                                                    const std::string& filterSpec) const
{
    return impl_->engine.computeAnalysisById(analysisId, state, filterSpec);
}

}
