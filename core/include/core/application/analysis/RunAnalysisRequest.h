#pragma once

#include <string>

namespace core::application::analysis {

struct RunAnalysisRequest {
    std::string analysisId;
    std::string filterSpec;
};

} // namespace core::application::analysis
