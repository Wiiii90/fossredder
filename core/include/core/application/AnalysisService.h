#pragma once

#include "core/analysis/AnalysisEngine.h"
#include "core/models/AnalysisResult.h"

#include <string>
#include <vector>

namespace core::domain {
class Analysis;
struct AppState;
}

namespace core::application {

class AnalysisService {
public:
    AnalysisResult computeAnalysisById(const core::domain::AppState& state,
                                       const std::string& analysisId,
                                       const std::string& filterSpec = {}) const;

    std::vector<std::string> contractTypes(const core::domain::AppState& state) const;

private:
    core::analysis::AnalysisEngine engine_;
};

}
