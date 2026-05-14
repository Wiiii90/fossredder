#pragma once

#include <string>

namespace core::application::analysis {

struct AnalysisRequest {
    std::string analysisId;
    std::string filterSpecification;

    [[nodiscard]] bool empty() const noexcept {
        return analysisId.empty() && filterSpecification.empty();
    }
};

} // namespace core::application::analysis
